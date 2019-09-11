// hello_vrs.cpp : Defines the entry point for the console application.
//
#include <tobii/tobii.h>
#include <tobii/tobii_streams.h>
#include <random>

#include <vector>
#include <array>

#include <glm/gtc/matrix_inverse.hpp>

#include <iostream>
#include <fstream>

#include "cg_base.h"
#include "vulkan_render_object.h"
#include "vulkan_texture.h"
#include "vulkan_command_buffer_manager.h"
#include "vulkan_drawer.h"
#include "vulkan_image_presenter.h"
#include "vulkan_render_queue.h"
#include "vulkan_renderer.h"
#include "vulkan_pipeline.h"
#include "vulkan_framebuffer.h"
#include "vulkan_resource_bundle_layout.h"
#include "vulkan_resource_bundle_group.h"
#include "vulkan_resource_bundle.h"
#include "vulkan_memory_manager.h"

#include "vrs_image_compute_drawer.h"
#include "vrs_cas_compute_drawer.h"
#include "vrs_cas_edge_compute_drawer.h"
#include "vrs_mas_compute_drawer.h"
#include "vrs_eye_tracked_blit.h"

#include "eyetracking_interface.h"

#include "model.h"
#include "AmbientLight.h"
#include "DirectionalLight.h"
#include "PointLight.h"
#include "common_structs.h"

#include "deferred_renderer.h"

#include "cp_interpolation.h"
#include "bezier_curve.h"
#include "catmull_rom_spline.h"
#include "quadratic_uniform_b_spline.h"
#include "cubic_uniform_b_spline.h"

#define VRS_EYE 0
#define VRS_EYE_BLIT 0
#define VRS_CAS 0
#define VRS_CAS_EDGE 1
#define VRS_MAS 0
#define VRS_TAA 0

#define DEFERRED_SHADING 1

#define BLIT_FINAL_IMAGE 0
#define TAA_ENABLED 0


const int MAX_COUNT_POINT_LIGHTS = 100;

const std::string TEXTURE_PATH = "assets/chalet/chalet.jpg";



class camera_path : public cgb::cg_element
{
public:
	camera_path(cgb::quake_camera& cam)
		: mCam{ &cam } // Target camera
		, mSpeed{ 0.1f } // How fast does it move
		, mAutoPathActive{ true }
	{
		// The path to follow
		mPath = std::make_unique<cgb::quadratic_uniform_b_spline>(std::vector<glm::vec3>{
			glm::vec3(10.0f, 15.0f, -10.0f),
				glm::vec3(-10.0f, 5.0f, -5.0f),
				glm::vec3(0.0f, 0.0f, 0.0f),
				glm::vec3(10.0f, 15.0f, 10.0f),
				glm::vec3(10.0f, 10.0f, 0.0f)
		});
	}

	int32_t priority() const override
	{
		return 5;
	}

	void initialize() override
	{
		mStartTime = cgb::time().time_since_start();
	}

	void update() override
	{
		// [Space] ... toggle auto-path
		if (cgb::input().key_pressed(cgb::key_code::space)) {
			mAutoPathActive = !mAutoPathActive;
		}
		// [R] ... reset animation:
		if (cgb::input().key_pressed(cgb::key_code::r)) {
			resetAnimation();
		}

		// Animate along the curve:
		if (mAutoPathActive) {
			auto t = computeT();
			if (t >= 0.00001 && t <= 1) {
				mCam->set_translation(mPath->value_at(t));
				mCam->look_along(mPath->slope_at(t));
			}
		}
	}

	void resetAnimation() {
		mAutoPathActive = true;
		mStartTime = cgb::time().time_since_start();
	}

	bool animationFinished() {
		auto t = computeT();
		return (t >= 1.0);
	}

private:
	cgb::quake_camera* mCam;
	float mStartTime;
	float mSpeed;
	bool mAutoPathActive;
	std::unique_ptr<cgb::cp_interpolation> mPath;

	float computeT() {
		return (cgb::time().time_since_start() - mStartTime) * mSpeed;
	}
};

class vrs_behavior : public cgb::cg_element
{

public:
	cgb::quake_camera mCamera;
	camera_path mCameraPath;

	vrs_behavior() : m_ambient_light(glm::vec3(1 / 255.0f, 2 / 255.0f, 3 / 255.0f)), m_dir_light(glm::vec3(13 / 255.0f, 17 / 255.0f, 27 / 255.0f), glm::vec3(-0.38f, -0.78f, -0.49f)), mCameraPath(mCamera) {
	}
private:

	vk::DescriptorSetLayout vrsComputeDescriptorSetLayout;
	vk::DescriptorPool vrsComputeDescriptorPool;
	std::vector<vk::DescriptorSet> mVrsComputeDescriptorSets;

	vk::CommandPool commandPool;
	vk::CommandPool transferCommandPool;

	bool framebufferResized = false;

	static void framebufferResizeCallback(GLFWwindow* window, int width, int height)
	{
		auto app = reinterpret_cast<vrs_behavior*>(glfwGetWindowUserPointer(window));
		app->framebufferResized = true;
	}

	std::shared_ptr<cgb::vulkan_render_object> mVrsDebugFullscreenQuad;
	std::shared_ptr<cgb::vulkan_texture> texture;
	std::shared_ptr<cgb::vulkan_image> textureImage;
	std::shared_ptr<cgb::vulkan_command_buffer_manager> drawCommandBufferManager;
	std::shared_ptr<cgb::vulkan_command_buffer_manager> transferCommandBufferManager;
	std::unique_ptr<cgb::vulkan_drawer> drawer;
	std::shared_ptr<vrs_image_compute_drawer> mVrsImageComputeDrawer;
	std::shared_ptr<vrs_eye_tracked_blit> mVrsEyeTrackedBlitDrawer;

	// render target needed for MSAA
	std::shared_ptr<cgb::vulkan_image> colorImage;
	std::shared_ptr<cgb::vulkan_image> depthImage;
	std::vector<std::shared_ptr<cgb::vulkan_image>> vrsImages;
	std::vector<std::shared_ptr<cgb::vulkan_image>> vrsDefaultImage;
	std::vector<std::shared_ptr<cgb::vulkan_image>> vrsDebugImages;
	std::vector<std::shared_ptr <cgb::vulkan_texture>> vrsDebugTextureImages;
	std::shared_ptr<cgb::vulkan_image_presenter> mImagePresenter;
	std::shared_ptr<cgb::vulkan_render_queue> mVulkanRenderQueue;
	std::shared_ptr<cgb::vulkan_renderer> mRenderer;
	std::shared_ptr<cgb::vulkan_renderer> mVrsRenderer;
	std::shared_ptr<cgb::vulkan_renderer> mFinalBlitRenderer;
	std::shared_ptr<cgb::vulkan_pipeline> mRenderVulkanPipeline;
	std::shared_ptr<cgb::vulkan_pipeline> mComputeVulkanPipeline;
	std::shared_ptr<cgb::vulkan_framebuffer> mVulkanFramebuffer;
	std::shared_ptr<cgb::vulkan_resource_bundle_layout> mResourceBundleLayout;
	std::shared_ptr<cgb::vulkan_resource_bundle_group> mResourceBundleGroup;

	std::shared_ptr<eyetracking_interface> eyeInf;

	std::unique_ptr<cgb::Model> mSponzaModel;
	std::unique_ptr<cgb::Model> mParallelPipesModel;

	std::shared_ptr<cgb::vulkan_resource_bundle_layout> mGlobalResourceBundleLayout; // contains lights and global flags
	std::shared_ptr<cgb::vulkan_resource_bundle> mGlobalResourceBundle;

	std::shared_ptr<cgb::vulkan_resource_bundle_layout> mMaterialResourceBundleLayout; // contains material properties staying same for all objects
	std::shared_ptr<cgb::vulkan_resource_bundle_layout> mMaterialObjectResourceBundleLayout; // contains material properties varying for each object


	cgb::AmbientLight m_ambient_light;
	cgb::DirectionalLight m_dir_light;
	std::vector<cgb::PointLight> mPointLights;

	struct PointLights
	{
		cgb::PointLightGpuData pointLightData[MAX_COUNT_POINT_LIGHTS];
		int count;
	};

	std::vector<std::shared_ptr<cgb::vulkan_buffer>> mAmbientLightBuffers;
	std::vector<std::shared_ptr<cgb::vulkan_buffer>> mDirLightBuffers;
	std::vector<std::shared_ptr<cgb::vulkan_buffer>> mPointLightsBuffers;

	std::unique_ptr<cgb::vulkan_drawer> mMaterialDrawer;
	std::shared_ptr<cgb::vulkan_pipeline> mMaterialPipeline;

	std::unique_ptr<cgb::vulkan_drawer> mVrsDebugDrawer;
	std::shared_ptr<cgb::vulkan_pipeline> mVrsDebugPipeline;

	// VRS: content adaptive shading
	std::shared_ptr<cgb::vulkan_image> mVrsPrevRenderMsaaImage;
	std::vector<std::shared_ptr<cgb::vulkan_image>> mVrsPrevRenderImages;
	std::vector<std::shared_ptr<cgb::vulkan_texture>> mVrsPrevRenderTextures;
	std::vector<std::shared_ptr<cgb::vulkan_image>> mVrsPrevRenderBlitImages;
	std::vector<std::shared_ptr<cgb::vulkan_texture>> mVrsPrevRenderBlitTextures;

	std::shared_ptr<vrs_cas_compute_drawer> mVrsCasComputeDrawer;
	std::shared_ptr<cgb::vulkan_pipeline> mVrsCasComputePipeline;

	std::shared_ptr<vrs_cas_edge_compute_drawer> mVrsCasEdgeComputeDrawer;
	std::shared_ptr<cgb::vulkan_pipeline> mVrsCasEdgeComputePipeline;
	std::vector<std::shared_ptr<cgb::vulkan_image>> mVrsEdgeImages;
	std::vector<std::shared_ptr<cgb::vulkan_image>> mVrsEdgeBlitImages;
	std::vector<std::shared_ptr<cgb::vulkan_texture>> mVrsEdgeBlitTextures;

	// VRS: motion adaptive shading
	std::shared_ptr<vrs_mas_compute_drawer> mVrsMasComputeDrawer;
	std::shared_ptr<cgb::vulkan_pipeline> mVrsMasComputePipeline;

	std::vector<std::shared_ptr<cgb::vulkan_image>> mVrsMasMotionVecBlitImages;
	std::vector<std::shared_ptr<cgb::vulkan_texture>> mVrsMasMotionVecBlitTextures;

	// Post Processing (currently only passthrough into swapchain image)
	std::shared_ptr<cgb::vulkan_framebuffer> mPostProcFramebuffer;
	std::unique_ptr<cgb::vulkan_drawer> mPostProcDrawer;
	std::shared_ptr<cgb::vulkan_pipeline> mPostProcPipeline;
	std::shared_ptr<cgb::vulkan_renderer> mPostProcRenderer;

	std::vector<std::shared_ptr<cgb::vulkan_image>> mPostProcImages;
	std::vector<std::shared_ptr<cgb::vulkan_texture>> mPostProcTextures;
	std::array<std::shared_ptr<cgb::vulkan_render_object>, 2> mPostProcFullScreenQuads;

	// TAA
	std::vector<int> mTAAIndices;

	std::unique_ptr<cgb::vulkan_drawer> mTAADrawer;
	std::shared_ptr<cgb::vulkan_pipeline> mTAAPipeline;
	std::shared_ptr<cgb::vulkan_renderer> mTAARenderer;

	std::array<std::shared_ptr<cgb::vulkan_framebuffer>, 2> mTAAFramebuffers;

	std::array<std::vector<std::shared_ptr<cgb::vulkan_image>>, 2> mTAAImages;
	std::array<std::vector<std::shared_ptr<cgb::vulkan_texture>>, 2> mTAATextures;
	std::array<std::shared_ptr<cgb::vulkan_render_object>, 2> mTAAFullScreenQuads;

	std::unique_ptr<cgb::vulkan_drawer> mTAAMeanVarianceDrawer;
	std::shared_ptr<cgb::vulkan_pipeline> mTAAMeanVariancePipeline;
	std::shared_ptr<cgb::vulkan_renderer> mTAAMeanVarianceRenderer;

	std::shared_ptr<cgb::vulkan_framebuffer> mTAAMeanVarianceFramebuffer;

	std::vector<std::shared_ptr<cgb::vulkan_image>> mTAAMeanVarianceImages;
	std::vector<std::shared_ptr<cgb::vulkan_texture>> mTAAMeanVarianceTextures;
	std::vector<std::shared_ptr<cgb::vulkan_image>> mTAAVariance2Images;
	std::vector<std::shared_ptr<cgb::vulkan_texture>> mTAAVariance2Textures;

	std::vector<glm::vec2> jitter;


	std::shared_ptr<cgb::vulkan_image> mMotionVectorMsaaImage;
	std::vector<std::shared_ptr<cgb::vulkan_image>> mMotionVectorImages;
	std::vector<std::shared_ptr<cgb::vulkan_texture>> mMotionVectorTextures;

	std::vector<std::shared_ptr<prev_frame_data>> mPrevFrameData;

	std::shared_ptr<deferred_renderer> mDefRend;


	// VRS: TAA based shading rate

	std::shared_ptr<vrs_mas_compute_drawer> mVrsTAAClipComputeDrawer;
	std::shared_ptr<cgb::vulkan_pipeline> mVrsTAAClipComputePipeline;

	std::vector<std::shared_ptr<cgb::vulkan_image>> mVrsTAAClipImages;
	std::vector<std::shared_ptr<cgb::vulkan_image>> mVrsTAAClipBlitImages;
	std::vector<std::shared_ptr<cgb::vulkan_texture>> mVrsTAAClipBlitTextures;

	// super sampling
	uint32_t renderWidth;
	uint32_t renderHeight;
	double renderResMultiplier = 1;


	// capture frame times
	struct frame_time_data {
		unsigned int frame_count;
		float sum_t;
		float current_delta;
	};

	bool captureFrameTime = false;
	bool captureFrameTimeReset = false;
	std::vector<frame_time_data> frameTimeDataVector;

	std::shared_ptr<vrs_image_compute_drawer_base> mCurrentVrsImageComputeDrawer;


public:
	void initialize() override
	{
		eyeInf = std::make_shared<eyetracking_interface>();
		//initWindow();

		auto window = cgb::context().main_window()->handle()->mHandle;
		glfwSetWindowUserPointer(window, this);
		glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
		initVulkan();
	}

	void finalize() override
	{
		// wait for all commands to complete
		cgb::vulkan_context::instance().device.waitIdle();

		cleanup();
	}

	void update() override
	{
		if (cgb::input().key_pressed(cgb::key_code::escape)) {
			cgb::current_composition().stop();
		}

		if (mImagePresenter->is_swap_chain_recreated() || cgb::input().key_pressed(cgb::key_code::f4) || framebufferResized) {
			framebufferResized = false;
			recreateSwapChain();
			mImagePresenter->reset_swap_chain_recreated();
		}
		if (cgb::input().key_pressed(cgb::key_code::f5)) {
			cgb::vulkan_context::instance().device.waitIdle();
			mVrsDebugPipeline->bake();
			mMaterialPipeline->bake();
			mRenderVulkanPipeline->bake();

			if (cgb::vulkan_context::instance().shadingRateImageSupported) {
				mComputeVulkanPipeline->bake();
				mVrsCasComputePipeline->bake();
				mVrsCasEdgeComputePipeline->bake();
				mVrsMasComputePipeline->bake();
				mVrsTAAClipComputePipeline->bake();
			}

			mPostProcPipeline->bake();
			mTAAMeanVariancePipeline->bake();
			mTAAPipeline->bake();

#if DEFERRED_SHADING
			mDefRend->reload_shaders();
#endif
		}
		if (cgb::input().key_pressed(cgb::key_code::f7)) {
			start_capture_frame_data();
		}
		if (cgb::input().key_pressed(cgb::key_code::f8)) {
			end_capture_frame_data();
		}

		// EVALUATION
		static int shadingRateStrategyIndex = -2;
		static int resolutionStrategyIndex = 0;
		static bool evalStarted = false;
		static std::string title = "NONE";
		static int msaaSamples = 2;

		if (evalStarted && !cgb::vulkan_context::instance().shadingRateImageSupported && shadingRateStrategyIndex == -2) {
			mCameraPath.resetAnimation();
			start_capture_frame_data();
			shadingRateStrategyIndex += 1;
		}

		if (evalStarted && mCameraPath.animationFinished()) {
			if (!cgb::vulkan_context::instance().shadingRateImageSupported) {
				cgb::vulkan_context::instance().shadingRateImageSupported = true;
				recreateSwapChain();
				return;
			}

			end_capture_frame_data("msaa" + std::to_string((int)cgb::vulkan_context::instance().msaaSamples) + "_res" + std::to_string(renderWidth) + "_" + std::to_string(renderHeight), title + "\n");
			mCameraPath.resetAnimation();
			shadingRateStrategyIndex += 1;

			if (shadingRateStrategyIndex < 6) {
				start_capture_frame_data();

				if (cgb::vulkan_context::instance().shadingRateImageSupported) {
					// VRS_EYE
					if (shadingRateStrategyIndex == 0) {
						mCurrentVrsImageComputeDrawer = mVrsImageComputeDrawer;
						title = "VRS_EYE";
					}
					// VRS_EYE_BLIT
					if (shadingRateStrategyIndex == 1) {
						mCurrentVrsImageComputeDrawer = mVrsEyeTrackedBlitDrawer;
						title = "VRS_EYE_BLIT";
					}
					// VRS_CAS
					else if (shadingRateStrategyIndex == 2) {
						mCurrentVrsImageComputeDrawer = mVrsCasComputeDrawer;
						title = "VRS_CAS";
					}
					// VRS_CAS_EDGE
					else if (shadingRateStrategyIndex == 3) {
						mCurrentVrsImageComputeDrawer = mVrsCasEdgeComputeDrawer;
						title = "VRS_CAS_EDGE";
					}
					// VRS_MAS
					else if (shadingRateStrategyIndex == 4) {
						mCurrentVrsImageComputeDrawer = mVrsMasComputeDrawer;
						title = "VRS_MAS";
					}
					// VRS_TAA
					else if (shadingRateStrategyIndex == 5) {
						mCurrentVrsImageComputeDrawer = mVrsTAAClipComputeDrawer;
						title = "VRS_TAA";
					}
				}
			}
			else {
				resolutionStrategyIndex += 1;
				if (resolutionStrategyIndex < 4) {
					switch (resolutionStrategyIndex) {
					case 1:
						renderResMultiplier = 4.0 / 3.0;
						break;
					case 2:
						renderResMultiplier = 2;
						break;
					case 3:
						renderResMultiplier = 4;
						break;
					}
					// reset values for 
					shadingRateStrategyIndex = -2;
					title = "NONE";

					cgb::vulkan_context::instance().shadingRateImageSupported = false;
					recreateSwapChain();
				}
				else {

					if ((int)cgb::vulkan_context::instance().msaaSamples <= 8) {
						cgb::vulkan_context::instance().msaaSamples = (vk::SampleCountFlagBits)((int)cgb::vulkan_context::instance().msaaSamples * 2);
						// reset values for 
						resolutionStrategyIndex = 0;
						shadingRateStrategyIndex = -2;
						title = "NONE";

						cgb::vulkan_context::instance().shadingRateImageSupported = false;
						recreateSwapChain();
					}
					else {
						evalStarted = false;
					}
				}
			}
		}

		if (cgb::input().key_pressed(cgb::key_code::f9)) {
			evalStarted = true;
			shadingRateStrategyIndex = -2;
			resolutionStrategyIndex = 0;
			title = "NONE";
			renderResMultiplier = 1.0;
			cgb::vulkan_context::instance().msaaSamples = vk::SampleCountFlagBits::e1;

			cgb::vulkan_context::instance().shadingRateImageSupported = false;
			recreateSwapChain();
		}

		// END EVALUATION

		if (cgb::input().key_pressed(cgb::key_code::tab)) {
			if (mCamera.is_enabled()) {
				mCamera.disable();
			}
			else {
				mCamera.enable();
			}
		}

		auto time = cgb::time().absolute_time();

		const auto kSpeed = 10.0f;
		const auto kDistanceX = -0.23f;
		const auto kDistanceY = 1.0f;
		mParallelPipesModel->mesh_at(0).m_scene_transformation_matrix = glm::translate(glm::vec3(
			kDistanceX * glm::sin(kSpeed * time),
			kDistanceY * glm::sin(kSpeed * time),
			0.0f));
	}

	void render() override
	{
		static float sum_t = 0.0f;
		static unsigned int frame_count = 0;

		auto eyeData = eyeInf->get_eyetracking_data();

		drawFrame();
		frame_count++;

		sum_t += cgb::time().delta_time();

		if (captureFrameTimeReset) {
			captureFrameTimeReset = false;
			sum_t = 0;
			frame_count = 0;
		}
		if (sum_t >= 1.0f) {
			cgb::context().main_window()->set_title(std::to_string(1.0f / cgb::time().delta_time()).c_str());
			if (captureFrameTime) {
				frameTimeDataVector.push_back({frame_count, sum_t, cgb::time().delta_time() });
			}
			sum_t = 0.0f;
			frame_count = 0;

			printf("Gaze point: %f, %f\n",
				eyeData.positionX,
				eyeData.positionY);
		}
	}

private:
	void initVulkan()
	{
		cgb::vulkan_context::instance().initVulkan();

		createCommandPools();

		transferCommandBufferManager = std::make_shared<cgb::vulkan_command_buffer_manager>(transferCommandPool, cgb::vulkan_context::instance().graphicsQueue);
		cgb::vulkan_context::instance().transferCommandBufferManager = transferCommandBufferManager;
		mImagePresenter = std::make_shared<cgb::vulkan_image_presenter>(cgb::vulkan_context::instance().presentQueue, cgb::vulkan_context::instance().surface, cgb::vulkan_context::instance().findQueueFamilies());
		init_swapchain_objects();

		cgb::vulkan_context::instance().msaaSamples = vk::SampleCountFlagBits::e1;
	}

	void init_swapchain_objects()
	{
		renderWidth = mImagePresenter->get_swap_chain_extent().width * renderResMultiplier;
		renderHeight = mImagePresenter->get_swap_chain_extent().height * renderResMultiplier;

		cgb::vulkan_context::instance().dynamicRessourceCount = mImagePresenter->get_swap_chain_images_count();
		drawCommandBufferManager = std::make_shared<cgb::vulkan_command_buffer_manager>(cgb::vulkan_context::instance().dynamicRessourceCount, commandPool, cgb::vulkan_context::instance().graphicsQueue);
		mVulkanRenderQueue = std::make_shared<cgb::vulkan_render_queue>(cgb::vulkan_context::instance().graphicsQueue);

		std::vector<std::shared_ptr<cgb::vulkan_renderer>> dependentRenderers = {};
		if (cgb::vulkan_context::instance().shadingRateImageSupported) {
			mVrsRenderer = std::make_shared<cgb::vulkan_renderer>(nullptr, mVulkanRenderQueue, drawCommandBufferManager, std::vector<std::shared_ptr<cgb::vulkan_renderer>>{}, true);
			dependentRenderers.push_back(mVrsRenderer);
		}
		mRenderer = std::make_shared<cgb::vulkan_renderer>(nullptr, mVulkanRenderQueue, drawCommandBufferManager, dependentRenderers);
		mTAAMeanVarianceRenderer = std::make_shared<cgb::vulkan_renderer>(mImagePresenter, mVulkanRenderQueue, drawCommandBufferManager); // predecessors added later on
		mTAARenderer = std::make_shared<cgb::vulkan_renderer>(mImagePresenter, mVulkanRenderQueue, drawCommandBufferManager, std::vector<std::shared_ptr<cgb::vulkan_renderer>>{ mTAAMeanVarianceRenderer }); // predecessors added later on
		mFinalBlitRenderer = std::make_shared<cgb::vulkan_renderer>(mImagePresenter, mVulkanRenderQueue, drawCommandBufferManager, std::vector<std::shared_ptr<cgb::vulkan_renderer>>{ mTAARenderer }, true);
		mPostProcRenderer = std::make_shared<cgb::vulkan_renderer>(mImagePresenter, mVulkanRenderQueue, drawCommandBufferManager, std::vector<std::shared_ptr<cgb::vulkan_renderer>> { mTAARenderer });

		createColorResources();
		createDepthResources();
		if (cgb::vulkan_context::instance().shadingRateImageSupported) {
			createVRSImageResources();
		}
		createVRSTAAImageResources();

		mVulkanFramebuffer = std::make_shared<cgb::vulkan_framebuffer>(renderWidth, renderHeight,
			cgb::vulkan_context::instance().dynamicRessourceCount, cgb::vulkan_context::instance().msaaSamples);
		mVulkanFramebuffer->add_dynamic_color_attachment(colorImage, mPostProcImages, vk::ImageLayout::eShaderReadOnlyOptimal);
		mVulkanFramebuffer->set_depth_attachment(depthImage);
		if (cgb::vulkan_context::instance().shadingRateImageSupported) {
			mVulkanFramebuffer->add_dynamic_color_attachment(mVrsPrevRenderMsaaImage, mVrsPrevRenderImages, vk::ImageLayout::eShaderReadOnlyOptimal);
		}
		mVulkanFramebuffer->add_dynamic_color_attachment(mMotionVectorMsaaImage, mMotionVectorImages, vk::ImageLayout::eShaderReadOnlyOptimal);
		mVulkanFramebuffer->bake();
		mRenderer->set_framebuffer(mVulkanFramebuffer);

		createDescriptorSetLayout();
		mResourceBundleGroup = std::make_shared<cgb::vulkan_resource_bundle_group>();
		if (cgb::vulkan_context::instance().shadingRateImageSupported) {
			createVrsComputeDescriptorSetLayout();
		}

		vk::Viewport viewport = {};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = (float)renderWidth;
		viewport.height = (float)renderHeight;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		vk::Viewport postProcViewport = {};
		postProcViewport.x = 0.0f;
		postProcViewport.y = 0.0f;
		postProcViewport.width = (float)mImagePresenter->get_swap_chain_extent().width;
		postProcViewport.height = (float)mImagePresenter->get_swap_chain_extent().height;
		postProcViewport.minDepth = 0.0f;
		postProcViewport.maxDepth = 1.0f;

		vk::Rect2D scissor = {};
		scissor.offset = { 0, 0 };
		scissor.extent = vk::Extent2D{renderWidth, renderHeight};
		
		vk::Rect2D postProcScissor = {};
		postProcScissor.offset = { 0, 0 };
		postProcScissor.extent = vk::Extent2D{ mImagePresenter->get_swap_chain_extent().width, mImagePresenter->get_swap_chain_extent().height };

		//Atribute description bindings
		auto bind1 = std::make_shared<cgb::vulkan_attribute_description_binding>(1, sizeof(Vertex), vk::VertexInputRate::eVertex);
		bind1->add_attribute_description(0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, pos));
		bind1->add_attribute_description(1, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, color));
		auto bind2 = std::make_shared<cgb::vulkan_attribute_description_binding>(2, sizeof(Vertex), vk::VertexInputRate::eVertex);
		bind2->add_attribute_description(2, vk::Format::eR32G32Sfloat, offsetof(Vertex, texCoord));

		// Render Drawer and Pipeline
		mRenderVulkanPipeline = std::make_shared<cgb::vulkan_pipeline>(mVulkanFramebuffer->get_render_pass(), viewport, scissor, cgb::vulkan_context::instance().msaaSamples, std::vector<std::shared_ptr<cgb::vulkan_resource_bundle_layout>> { mResourceBundleLayout }, sizeof(PushUniforms));

		mRenderVulkanPipeline->add_attr_desc_binding(bind1);
		mRenderVulkanPipeline->add_attr_desc_binding(bind2);
		mRenderVulkanPipeline->add_color_blend_attachment_state(mRenderVulkanPipeline->get_color_blend_attachment_state(0));
		mRenderVulkanPipeline->add_color_blend_attachment_state(mRenderVulkanPipeline->get_color_blend_attachment_state(0));
		mRenderVulkanPipeline->add_shader(cgb::ShaderStageFlagBits::eVertex, "shaders/triangle.vert.spv");
		mRenderVulkanPipeline->add_shader(cgb::ShaderStageFlagBits::eFragment, "shaders/triangle.frag.spv");
		mRenderVulkanPipeline->disable_shading_rate_image();

		mRenderVulkanPipeline->bake();
		drawer = std::make_unique<cgb::vulkan_drawer>(drawCommandBufferManager, mRenderVulkanPipeline);

		createTexture();

		auto vrsCasResourceBundleLayout = std::make_shared<cgb::vulkan_resource_bundle_layout>();
		vrsCasResourceBundleLayout->add_binding(0, vk::DescriptorType::eStorageImage, cgb::ShaderStageFlagBits::eCompute);
		//vrsCasResourceBundleLayout->add_binding(1, vk::DescriptorType::eStorageImage, cgb::ShaderStageFlagBits::eCompute);
		vrsCasResourceBundleLayout->add_binding(1, vk::DescriptorType::eCombinedImageSampler, cgb::ShaderStageFlagBits::eCompute);
		vrsCasResourceBundleLayout->add_binding(2, vk::DescriptorType::eCombinedImageSampler, cgb::ShaderStageFlagBits::eCompute); // only needed for VRS_CAS_EDGE
		vrsCasResourceBundleLayout->bake();
		auto vrsCasResourceBundle = mResourceBundleGroup->create_resource_bundle(vrsCasResourceBundleLayout, true);
		if (cgb::vulkan_context::instance().shadingRateImageSupported) {
			vrsCasResourceBundle->add_dynamic_image_resource(0, vk::ImageLayout::eGeneral, vrsImages);
			//vrsCasResourceBundle->add_dynamic_image_resource(1, vk::ImageLayout::eGeneral, mVrsPrevRenderBlitImages);
			vrsCasResourceBundle->add_dynamic_image_resource(1, vk::ImageLayout::eGeneral, mVrsPrevRenderBlitTextures);
			vrsCasResourceBundle->add_dynamic_image_resource(2, vk::ImageLayout::eGeneral, mVrsEdgeBlitTextures);  // only needed for VRS_CAS_EDGE
		}

		auto vrsMasResourceBundleLayout = std::make_shared<cgb::vulkan_resource_bundle_layout>();
		vrsMasResourceBundleLayout->add_binding(0, vk::DescriptorType::eStorageImage, cgb::ShaderStageFlagBits::eCompute);
		vrsMasResourceBundleLayout->add_binding(1, vk::DescriptorType::eCombinedImageSampler, cgb::ShaderStageFlagBits::eCompute);
		vrsMasResourceBundleLayout->add_binding(2, vk::DescriptorType::eCombinedImageSampler, cgb::ShaderStageFlagBits::eCompute);
		vrsMasResourceBundleLayout->bake();
		auto vrsMasResourceBundle = mResourceBundleGroup->create_resource_bundle(vrsMasResourceBundleLayout, true);
		auto vrsTAAClipResourceBundle = mResourceBundleGroup->create_resource_bundle(vrsMasResourceBundleLayout, true);
		if (cgb::vulkan_context::instance().shadingRateImageSupported) {
			vrsMasResourceBundle->add_dynamic_image_resource(0, vk::ImageLayout::eGeneral, vrsImages);
			vrsMasResourceBundle->add_dynamic_image_resource(1, vk::ImageLayout::eGeneral, mVrsMasMotionVecBlitTextures);
			vrsMasResourceBundle->add_dynamic_image_resource(2, vk::ImageLayout::eGeneral, mVrsPrevRenderBlitTextures);

			vrsTAAClipResourceBundle->add_dynamic_image_resource(0, vk::ImageLayout::eGeneral, vrsImages);
			vrsTAAClipResourceBundle->add_dynamic_image_resource(1, vk::ImageLayout::eGeneral, mVrsTAAClipBlitTextures);
			vrsTAAClipResourceBundle->add_dynamic_image_resource(2, vk::ImageLayout::eGeneral, mVrsPrevRenderBlitTextures);
		}

		auto vrsDebugResourceBundleLayout = std::make_shared<cgb::vulkan_resource_bundle_layout>();
		vrsDebugResourceBundleLayout->add_binding(0, vk::DescriptorType::eStorageImage, cgb::ShaderStageFlagBits::eCompute);
		vrsDebugResourceBundleLayout->bake();
		auto vrsDebugResourceBundle = mResourceBundleGroup->create_resource_bundle(vrsDebugResourceBundleLayout, true);
		if (cgb::vulkan_context::instance().shadingRateImageSupported) {
			vrsDebugResourceBundle->add_dynamic_image_resource(0, vk::ImageLayout::eGeneral, vrsDebugImages);
		}

		if (cgb::vulkan_context::instance().shadingRateImageSupported) {
			createVrsComputeDescriptorPool();
			createVrsDescriptorSets();
			
			drawer->set_vrs_images(vrsImages);

			// Compute Drawer and Pipeline
			mComputeVulkanPipeline = std::make_shared<cgb::vulkan_pipeline>(std::vector<std::shared_ptr<cgb::vulkan_resource_bundle_layout>> {}, sizeof(vrs_eye_comp_data));
			mComputeVulkanPipeline->add_shader(cgb::ShaderStageFlagBits::eCompute, "shaders/vrs_img.comp.spv");
			mComputeVulkanPipeline->add_resource_bundle_layout(std::make_shared<cgb::vulkan_resource_bundle_layout>(vrsComputeDescriptorSetLayout));

			mVrsImageComputeDrawer = std::make_shared<vrs_image_compute_drawer>(drawCommandBufferManager, mComputeVulkanPipeline, vrsDebugImages);
			mVrsImageComputeDrawer->set_vrs_images(vrsImages);
			mVrsImageComputeDrawer->set_descriptor_sets(mVrsComputeDescriptorSets);
			mVrsImageComputeDrawer->set_width_height(vrsImages[0]->get_width(), vrsImages[0]->get_height());
			mVrsImageComputeDrawer->set_eye_inf(eyeInf);
			
			mVrsEyeTrackedBlitDrawer = std::make_shared<vrs_eye_tracked_blit>(drawCommandBufferManager, mComputeVulkanPipeline, vrsDebugImages);
			mVrsEyeTrackedBlitDrawer->set_vrs_images(vrsImages);
			mVrsEyeTrackedBlitDrawer->set_descriptor_sets(mVrsComputeDescriptorSets);
			mVrsEyeTrackedBlitDrawer->set_width_height(vrsImages[0]->get_width(), vrsImages[0]->get_height());
			mVrsEyeTrackedBlitDrawer->set_eye_inf(eyeInf);
			mVrsEyeTrackedBlitDrawer->precompute();

			// CAS
			mVrsCasComputePipeline = std::make_shared<cgb::vulkan_pipeline>(std::vector<std::shared_ptr<cgb::vulkan_resource_bundle_layout>> { vrsCasResourceBundleLayout, vrsDebugResourceBundleLayout }, sizeof(vrs_cas_comp_data));
			mVrsCasComputePipeline->add_shader(cgb::ShaderStageFlagBits::eCompute, "shaders/vrs_cas_img.comp.spv");
			mVrsCasComputePipeline->bake();

			mVrsCasComputeDrawer = std::make_shared<vrs_cas_compute_drawer>(drawCommandBufferManager, mVrsCasComputePipeline, std::vector<std::shared_ptr<cgb::vulkan_resource_bundle>> { vrsCasResourceBundle, vrsDebugResourceBundle },
				mVrsPrevRenderImages, mVrsPrevRenderBlitImages);
			mVrsCasComputeDrawer->set_vrs_images(vrsImages);
			mVrsCasComputeDrawer->set_width_height(vrsImages[0]->get_width(), vrsImages[0]->get_height());

			mVrsCasEdgeComputePipeline = std::make_shared<cgb::vulkan_pipeline>(std::vector<std::shared_ptr<cgb::vulkan_resource_bundle_layout>> { vrsCasResourceBundleLayout, vrsDebugResourceBundleLayout }, sizeof(vrs_cas_comp_data));
			mVrsCasEdgeComputePipeline->add_shader(cgb::ShaderStageFlagBits::eCompute, "shaders/vrs_cas_edge_img.comp.spv");
			mVrsCasEdgeComputePipeline->bake();
			mVrsCasEdgeComputeDrawer = std::make_shared<vrs_cas_edge_compute_drawer>(drawCommandBufferManager, mVrsCasEdgeComputePipeline, std::vector<std::shared_ptr<cgb::vulkan_resource_bundle>> { vrsCasResourceBundle, vrsDebugResourceBundle },
				mVrsPrevRenderImages, mVrsPrevRenderBlitImages, mVrsEdgeImages, mVrsEdgeBlitImages);
			mVrsCasEdgeComputeDrawer->set_vrs_images(vrsImages);
			mVrsCasEdgeComputeDrawer->set_width_height(vrsImages[0]->get_width(), vrsImages[0]->get_height());

			// MAS
			mVrsMasComputePipeline = std::make_shared<cgb::vulkan_pipeline>(std::vector<std::shared_ptr<cgb::vulkan_resource_bundle_layout>> { vrsMasResourceBundleLayout, vrsDebugResourceBundleLayout }, sizeof(vrs_cas_comp_data));
			mVrsMasComputePipeline->add_shader(cgb::ShaderStageFlagBits::eCompute, "shaders/vrs_mas_img.comp.spv");
			mVrsMasComputePipeline->bake();

			mVrsMasComputeDrawer = std::make_shared<vrs_mas_compute_drawer>(drawCommandBufferManager, mVrsMasComputePipeline, std::vector<std::shared_ptr<cgb::vulkan_resource_bundle>> { vrsMasResourceBundle, vrsDebugResourceBundle },
				mVrsPrevRenderImages, mVrsPrevRenderBlitImages, mMotionVectorImages, mVrsMasMotionVecBlitImages);
			mVrsMasComputeDrawer->set_vrs_images(vrsImages);
			mVrsMasComputeDrawer->set_width_height(vrsImages[0]->get_width(), vrsImages[0]->get_height());

			// VRS TAA
			mVrsTAAClipComputePipeline = std::make_shared<cgb::vulkan_pipeline>(std::vector<std::shared_ptr<cgb::vulkan_resource_bundle_layout>> { vrsMasResourceBundleLayout, vrsDebugResourceBundleLayout }, sizeof(vrs_cas_comp_data));
			mVrsTAAClipComputePipeline->add_shader(cgb::ShaderStageFlagBits::eCompute, "shaders/vrs_taa_based_img.comp.spv");
			mVrsTAAClipComputePipeline->bake();

			mVrsTAAClipComputeDrawer = std::make_shared<vrs_mas_compute_drawer>(drawCommandBufferManager, mVrsTAAClipComputePipeline, std::vector<std::shared_ptr<cgb::vulkan_resource_bundle>> { vrsTAAClipResourceBundle, vrsDebugResourceBundle },
				mVrsPrevRenderImages, mVrsPrevRenderBlitImages, mVrsTAAClipImages, mVrsTAAClipBlitImages);
			mVrsTAAClipComputeDrawer->set_vrs_images(vrsImages);
			mVrsTAAClipComputeDrawer->set_width_height(vrsImages[0]->get_width(), vrsImages[0]->get_height());
		}
		mVrsDebugFullscreenQuad = std::make_shared<cgb::vulkan_render_object>(verticesScreenQuad, indicesScreenQuad, mResourceBundleLayout, mResourceBundleGroup, texture, vrsDebugTextureImages);

		// post process objects
		auto postProcResourceBundleLayout = std::make_shared<cgb::vulkan_resource_bundle_layout>();
		postProcResourceBundleLayout->add_binding(0, vk::DescriptorType::eCombinedImageSampler, cgb::ShaderStageFlagBits::eFragment);
		postProcResourceBundleLayout->add_binding(1, vk::DescriptorType::eCombinedImageSampler, cgb::ShaderStageFlagBits::eFragment);
		postProcResourceBundleLayout->add_binding(2, vk::DescriptorType::eCombinedImageSampler, cgb::ShaderStageFlagBits::eFragment);
		postProcResourceBundleLayout->add_binding(3, vk::DescriptorType::eCombinedImageSampler, cgb::ShaderStageFlagBits::eFragment);
		postProcResourceBundleLayout->add_binding(4, vk::DescriptorType::eCombinedImageSampler, cgb::ShaderStageFlagBits::eFragment);
		postProcResourceBundleLayout->add_binding(5, vk::DescriptorType::eCombinedImageSampler, cgb::ShaderStageFlagBits::eFragment);
		postProcResourceBundleLayout->bake();
		auto postProcBundles = std::array<std::shared_ptr<cgb::vulkan_resource_bundle>, 2>{};
		for (int i = 0; i < postProcBundles.size(); i++) {
			postProcBundles[i] = mResourceBundleGroup->create_resource_bundle(postProcResourceBundleLayout, true);
		}

		// Post processing
		create_post_process_objects(postProcViewport, postProcScissor);

		// TAA
		create_TAA_objects(viewport, scissor, postProcResourceBundleLayout);

		postProcBundles[0]->add_dynamic_image_resource(0, vk::ImageLayout::eShaderReadOnlyOptimal, mPostProcTextures);
		postProcBundles[1]->add_dynamic_image_resource(0, vk::ImageLayout::eShaderReadOnlyOptimal, mPostProcTextures);
		// add previous frame image	
		postProcBundles[0]->add_dynamic_image_resource(1, vk::ImageLayout::eShaderReadOnlyOptimal, mTAATextures[1]);
		postProcBundles[1]->add_dynamic_image_resource(1, vk::ImageLayout::eShaderReadOnlyOptimal, mTAATextures[0]);

		postProcBundles[0]->add_dynamic_image_resource(2, vk::ImageLayout::eShaderReadOnlyOptimal, mMotionVectorTextures);
		postProcBundles[1]->add_dynamic_image_resource(2, vk::ImageLayout::eShaderReadOnlyOptimal, mMotionVectorTextures);
		postProcBundles[0]->add_dynamic_image_resource(3, vk::ImageLayout::eShaderReadOnlyOptimal, mVrsPrevRenderTextures);
		postProcBundles[1]->add_dynamic_image_resource(3, vk::ImageLayout::eShaderReadOnlyOptimal, mVrsPrevRenderTextures);
		postProcBundles[0]->add_dynamic_image_resource(4, vk::ImageLayout::eShaderReadOnlyOptimal, mTAAMeanVarianceTextures);
		postProcBundles[1]->add_dynamic_image_resource(4, vk::ImageLayout::eShaderReadOnlyOptimal, mTAAMeanVarianceTextures);
		postProcBundles[0]->add_dynamic_image_resource(5, vk::ImageLayout::eShaderReadOnlyOptimal, mTAAVariance2Textures);
		postProcBundles[1]->add_dynamic_image_resource(5, vk::ImageLayout::eShaderReadOnlyOptimal, mTAAVariance2Textures);


		for (int i = 0; i < mTAAFullScreenQuads.size(); i++) {
			mPostProcFullScreenQuads[i] = std::make_shared<cgb::vulkan_render_object>(verticesScreenQuad, indicesScreenQuad, std::vector<std::shared_ptr<cgb::vulkan_resource_bundle>> { postProcBundles[i] });
			mTAAFullScreenQuads[i] = std::make_shared<cgb::vulkan_render_object>(verticesScreenQuad, indicesScreenQuad, mResourceBundleLayout, mResourceBundleGroup, texture, mTAATextures[i]);
		}


		// initialize lights
		PointLights pointLights;
		create_lots_of_lights();
		for (int i = 0; i < MAX_COUNT_POINT_LIGHTS && i < mPointLights.size(); i++) {
			pointLights.pointLightData[i] = mPointLights[i].GetGpuData();
		}
		pointLights.count = mPointLights.size();

		auto dynResourceCount = cgb::vulkan_context::instance().dynamicRessourceCount;
		mAmbientLightBuffers.resize(dynResourceCount);
		mDirLightBuffers.resize(dynResourceCount);
		mPointLightsBuffers.resize(dynResourceCount);
		for (int i = 0; i < dynResourceCount; i++) {
			mAmbientLightBuffers[i] = std::make_shared<cgb::vulkan_buffer>(sizeof(cgb::AmbientLightGpuData), vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eUniformBuffer,
				vk::MemoryPropertyFlagBits::eDeviceLocal, &m_ambient_light.GetGpuData());
			mDirLightBuffers[i] = std::make_shared<cgb::vulkan_buffer>(sizeof(cgb::DirectionalLightGpuData), vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eUniformBuffer,
				vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, &m_dir_light.GetGpuData());
			mPointLightsBuffers[i] = std::make_shared<cgb::vulkan_buffer>(sizeof(PointLights), vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eUniformBuffer,
				vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, &pointLights);
		}

		mGlobalResourceBundleLayout = std::make_shared<cgb::vulkan_resource_bundle_layout>();
		mGlobalResourceBundleLayout->add_binding(0, vk::DescriptorType::eUniformBuffer, cgb::ShaderStageFlagBits::eFragment | cgb::ShaderStageFlagBits::eVertex);
		mGlobalResourceBundleLayout->add_binding(1, vk::DescriptorType::eUniformBuffer, cgb::ShaderStageFlagBits::eFragment | cgb::ShaderStageFlagBits::eVertex);
		mGlobalResourceBundleLayout->add_binding(2, vk::DescriptorType::eUniformBuffer, cgb::ShaderStageFlagBits::eFragment | cgb::ShaderStageFlagBits::eVertex);
		mGlobalResourceBundleLayout->bake();
		mGlobalResourceBundle = mResourceBundleGroup->create_resource_bundle(mGlobalResourceBundleLayout, true);
		mGlobalResourceBundle->add_dynamic_buffer_resource(0, mAmbientLightBuffers, sizeof(cgb::AmbientLightGpuData));
		mGlobalResourceBundle->add_dynamic_buffer_resource(1, mDirLightBuffers, sizeof(cgb::DirectionalLightGpuData));
		mGlobalResourceBundle->add_dynamic_buffer_resource(2, mPointLightsBuffers, sizeof(pointLights));

		// Sponza specific structures
		mMaterialObjectResourceBundleLayout = std::make_shared<cgb::vulkan_resource_bundle_layout>();
		mMaterialObjectResourceBundleLayout->add_binding(0, vk::DescriptorType::eUniformBuffer, cgb::ShaderStageFlagBits::eVertex | cgb::ShaderStageFlagBits::eFragment);
		mMaterialObjectResourceBundleLayout->bake();


		load_models();


		//Atribute description bindings
		auto& mesh = mSponzaModel->mesh_at(0);
		auto attrib_config = cgb::VertexAttribData::Nothing;
		attrib_config = attrib_config | cgb::VertexAttribData::Position;
		attrib_config = attrib_config | cgb::VertexAttribData::Tex2D;
		attrib_config = attrib_config | cgb::VertexAttribData::Normal;
		attrib_config = attrib_config | cgb::VertexAttribData::Tangents;
		attrib_config = attrib_config | cgb::VertexAttribData::Bitangents;

		auto sponzaBinding = std::shared_ptr<cgb::vulkan_attribute_description_binding>(new cgb::vulkan_attribute_description_binding(mesh.GetOrCreateForVertexAttribConfig(attrib_config)));

#if DEFERRED_SHADING

		// TODO move this huge constructor into struct for deferred shading
		mDefRend = std::make_shared<deferred_renderer>(dependentRenderers, mVulkanRenderQueue, drawCommandBufferManager, dynamic_image_resource{ colorImage, mPostProcImages },
			std::vector<dynamic_image_resource> { dynamic_image_resource{ mVrsPrevRenderMsaaImage, mVrsPrevRenderImages }, dynamic_image_resource{ mMotionVectorMsaaImage, mMotionVectorImages } },
			viewport, scissor, mMaterialObjectResourceBundleLayout, mGlobalResourceBundle, sponzaBinding, mResourceBundleGroup, vrsImages, mCamera);

		mTAAMeanVarianceRenderer->add_predecessors({ mDefRend->get_final_renderer() });
		mDefRend->allocate_resources();
#else
		mTAAMeanVarianceRenderer->add_predecessors({ mRenderer });
#endif // DEFERRED_SHADING

		mSponzaModel->allocate_render_object_data();
		mParallelPipesModel->allocate_render_object_data();

		for (int i = 0; i < mVrsDebugFullscreenQuad->get_resource_bundles().size(); i++) {
			mResourceBundleGroup->allocate_resource_bundle(mVrsDebugFullscreenQuad->get_resource_bundles()[i].get());
		}

		for (int j = 0; j < mTAAFullScreenQuads.size(); j++) {
			for (int i = 0; i < mPostProcFullScreenQuads[j]->get_resource_bundles().size(); i++) {
				mResourceBundleGroup->allocate_resource_bundle(mPostProcFullScreenQuads[j]->get_resource_bundles()[i].get());
			}
			for (int i = 0; i < mTAAFullScreenQuads[j]->get_resource_bundles().size(); i++) {
				mResourceBundleGroup->allocate_resource_bundle(mTAAFullScreenQuads[j]->get_resource_bundles()[i].get());
			}
		}


		mCamera.set_translation(glm::vec3(-0.67, 0.53, 6.07));
		//mCamera.LookAlong(glm::vec3(0.0f, 0.0f, -1.0f));
		mCamera.set_perspective_projection(glm::radians(60.0f), cgb::context().main_window()->aspect_ratio(), 0.1f, 100.0f);
		//mCamera.set_orthographic_projection(0, 30, 0, 30, 0.1f, 100.0f);
		//mQuakeCam.set_orthographic_projection(-5, 5, -5, 5, 0.5, 100);
		cgb::current_composition().add_element(mCamera);

		UniformBufferObject uboCam{};
		uboCam.view = mCamera.view_matrix();
		uboCam.proj = mCamera.projection_matrix();
		uboCam.mv = uboCam.view * mesh.transformation_matrix();
		uboCam.vmNormalMatrix = glm::mat4(glm::inverseTranspose(glm::mat3(uboCam.mv)));

		for (auto sponzaRenderObject : mSponzaModel->get_render_objects()) {
			for (int i = 0; i < cgb::vulkan_context::instance().dynamicRessourceCount; i++) {
				sponzaRenderObject->update_uniform_buffer(i, uboCam);
			}
		}
		for (auto sponzaRenderObject : mParallelPipesModel->get_render_objects()) {
			for (int i = 0; i < cgb::vulkan_context::instance().dynamicRessourceCount; i++) {
				sponzaRenderObject->update_uniform_buffer(i, uboCam);
			}
		}

		mResourceBundleGroup->allocate_resource_bundle(mGlobalResourceBundle.get());

		mMaterialPipeline = std::make_shared<cgb::vulkan_pipeline>(mVulkanFramebuffer->get_render_pass(), viewport, scissor, cgb::vulkan_context::instance().msaaSamples, std::vector<std::shared_ptr<cgb::vulkan_resource_bundle_layout>> { mGlobalResourceBundleLayout, cgb::MaterialData::get_resource_bundle_layout(), mMaterialObjectResourceBundleLayout }, sizeof(prev_frame_data), cgb::ShaderStageFlagBits::eVertex | cgb::ShaderStageFlagBits::eFragment);
		mMaterialPipeline->add_attr_desc_binding(sponzaBinding);
		mMaterialPipeline->add_color_blend_attachment_state(mRenderVulkanPipeline->get_color_blend_attachment_state(0));
		mMaterialPipeline->add_color_blend_attachment_state(mRenderVulkanPipeline->get_color_blend_attachment_state(0));
		mMaterialPipeline->add_shader(cgb::ShaderStageFlagBits::eVertex, "shaders/blinnphong_nm.vert.spv");
		mMaterialPipeline->add_shader(cgb::ShaderStageFlagBits::eFragment, "shaders/blinnphong_nm.frag.spv");
		mMaterialPipeline->bake();

		mMaterialDrawer = std::make_unique<cgb::vulkan_drawer>(drawCommandBufferManager, mMaterialPipeline, std::vector<std::shared_ptr<cgb::vulkan_resource_bundle>> { mGlobalResourceBundle });
		if (cgb::vulkan_context::instance().shadingRateImageSupported) {
			mMaterialDrawer->set_vrs_images(vrsImages);
		}

		// VRS Debug render, used with e.g. fullscreen quad
		mVrsDebugPipeline = std::make_shared<cgb::vulkan_pipeline>(mTAAFramebuffers[0]->get_render_pass(), viewport, scissor, vk::SampleCountFlagBits::e1, std::vector<std::shared_ptr<cgb::vulkan_resource_bundle_layout>> { mResourceBundleLayout }, sizeof(PushUniforms));
		mVrsDebugPipeline->add_attr_desc_binding(bind1);
		mVrsDebugPipeline->add_attr_desc_binding(bind2);
		mVrsDebugPipeline->add_shader(cgb::ShaderStageFlagBits::eVertex, "shaders/vrs_debug.vert.spv");
		mVrsDebugPipeline->add_shader(cgb::ShaderStageFlagBits::eFragment, "shaders/vrs_debug.frag.spv");
		//mVrsDebugPipeline->add_color_blend_attachment_state(mTAAMeanVariancePipeline->get_color_blend_attachment_state(0));

		auto& colorBlend = mVrsDebugPipeline->get_color_blend_attachment_state(0);
		colorBlend.blendEnable = VK_TRUE;
		colorBlend.dstColorBlendFactor = vk::BlendFactor::eOne;
		mVrsDebugPipeline->add_color_blend_attachment_state(colorBlend);
		//mVrsDebugPipeline->disable_shading_rate_image();
		mVrsDebugPipeline->bake();
		if (cgb::vulkan_context::instance().shadingRateImageSupported) {
			mVrsDebugDrawer = std::make_unique<cgb::vulkan_drawer>(drawCommandBufferManager, mVrsDebugPipeline);
			mVrsDebugDrawer->set_vrs_images(vrsImages);
			mResourceBundleGroup->allocate_resource_bundle(vrsCasResourceBundle.get());
			mResourceBundleGroup->allocate_resource_bundle(vrsMasResourceBundle.get());
			mResourceBundleGroup->allocate_resource_bundle(vrsTAAClipResourceBundle.get());
			mResourceBundleGroup->allocate_resource_bundle(vrsDebugResourceBundle.get());
		}

		mPrevFrameData.resize(cgb::vulkan_context::instance().dynamicRessourceCount);
		for (int i = 0; i < mPrevFrameData.size(); i++) {
			mPrevFrameData[i] = std::make_shared<prev_frame_data>();
		}


		if (cgb::vulkan_context::instance().shadingRateImageSupported) {
#if VRS_EYE
			mCurrentVrsImageComputeDrawer = mVrsImageComputeDrawer;
#elif VRS_EYE_BLIT
			mCurrentVrsImageComputeDrawer = mVrsEyeTrackedBlitDrawer;
#elif VRS_CAS
			mCurrentVrsImageComputeDrawer = mVrsCasComputeDrawer;
#elif VRS_CAS_EDGE
			mCurrentVrsImageComputeDrawer = mVrsCasEdgeComputeDrawer;
#elif VRS_MAS
			mCurrentVrsImageComputeDrawer = mVrsMasComputeDrawer;
#elif VRS_TAA
			mCurrentVrsImageComputeDrawer = mVrsTAAClipComputeDrawer;
#endif
		}
	}

	void cleanup()
	{
		cleanupSwapChain();

		transferCommandBufferManager.reset();
		mVulkanRenderQueue.reset();
		drawCommandBufferManager.reset();

		cgb::vulkan_context::instance().device.destroyCommandPool(transferCommandPool);
		cgb::vulkan_context::instance().device.destroyCommandPool(commandPool);

		// destroy instance (in context)
		mImagePresenter.reset();
	}

	//void initWindow()
	//{
	//	glfwInit();

	//	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	//	//glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	//	window = glfwCreateWindow(WIDTH, HEIGHT, "VulkanStSt", nullptr, nullptr);
	//	glfwSetWindowUserPointer(window, this);
	//	glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
	//}

	void cleanupSwapChain()
	{
		colorImage.reset();
		depthImage.reset();

		drawer.reset();
		if (cgb::vulkan_context::instance().shadingRateImageSupported) {
			mVrsEyeTrackedBlitDrawer.reset();
			mVrsImageComputeDrawer.reset();
			mVrsCasComputeDrawer.reset();
			mVrsCasEdgeComputeDrawer.reset();
			mVrsMasComputeDrawer.reset();
			mVrsTAAClipComputeDrawer.reset();
			mVrsDebugDrawer.reset();
			mCurrentVrsImageComputeDrawer.reset();
		}
		mMaterialDrawer.reset();
		mRenderVulkanPipeline.reset();
		mComputeVulkanPipeline.reset();
		mVrsCasComputePipeline.reset();
		mVrsCasComputePipeline.reset();
		mVrsMasComputePipeline.reset();
		mVrsTAAClipComputePipeline.reset();
		mVrsDebugPipeline.reset();

		mVulkanFramebuffer.reset();

		mPostProcDrawer.reset();
		mPostProcPipeline.reset();
		mPostProcFramebuffer.reset();

		mTAADrawer.reset();
		mTAAMeanVarianceDrawer.reset();
		mTAAPipeline.reset();
		mTAAMeanVariancePipeline.reset();
		for (int i = 0; i < mTAAFramebuffers.size(); i++) {
			mTAAFramebuffers[i].reset();
		}
		if (cgb::vulkan_context::instance().shadingRateImageSupported) {
			mVrsRenderer.reset();
		}
		mFinalBlitRenderer.reset();
		mRenderer.reset();
		mTAARenderer.reset();
		mTAAMeanVarianceRenderer.reset();
		mPostProcRenderer.reset();
#if DEFERRED_SHADING
		mDefRend.reset();
#endif // DEFERRED_SHADING

		// not part of the swapchain but free anyway
		mResourceBundleGroup.reset();

		if (cgb::vulkan_context::instance().shadingRateImageSupported) {
			cgb::vulkan_context::instance().device.destroyDescriptorPool(vrsComputeDescriptorPool);
		}

		mVrsDebugFullscreenQuad.reset();
		for (auto sponzaRenderObject : mSponzaModel->get_render_objects()) {
			sponzaRenderObject.reset();
		}
		for (auto sponzaRenderObject : mParallelPipesModel->get_render_objects()) {
			sponzaRenderObject.reset();
		}
		texture.reset();
		textureImage.reset();
		mSponzaModel.reset();
		mParallelPipesModel.reset();

		mVrsDebugFullscreenQuad.reset();
		mPostProcFullScreenQuads[0].reset();
		mPostProcFullScreenQuads[1].reset();
		mTAAFullScreenQuads[0].reset();
		mTAAFullScreenQuads[1].reset();
		mTAAFramebuffers[0].reset();
		mTAAFramebuffers[1].reset();
		mTAAMeanVarianceFramebuffer.reset();
		mVulkanFramebuffer.reset();

		// TODO remove, hack for 8k
		cgb::vulkan_context::instance().memoryManager->cleanup();
	}

	void recreateSwapChain()
	{
		mImagePresenter->recreate_swapchain();
		cleanupSwapChain();
		init_swapchain_objects();
	}

	void createCommandPools()
	{
		cgb::QueueFamilyIndices queueFamilyIndices = cgb::vulkan_context::instance().findQueueFamilies();

		vk::CommandPoolCreateInfo poolInfo = {};
		poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
		poolInfo.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer; // Optional

		if (cgb::vulkan_context::instance().device.createCommandPool(&poolInfo, nullptr, &commandPool) != vk::Result::eSuccess) {
			throw std::runtime_error("failed to create command pool!");
		}

		// create command pool for data transfers 
		vk::CommandPoolCreateInfo transferPoolInfo = {};
		transferPoolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
		transferPoolInfo.flags = vk::CommandPoolCreateFlagBits::eTransient; // Optional

		if (cgb::vulkan_context::instance().device.createCommandPool(&transferPoolInfo, nullptr, &transferCommandPool) != vk::Result::eSuccess) {
			throw std::runtime_error("failed to create command pool for data transfers!");
		}
	}

	void drawFrame()
	{
		static auto prevFrameModel = glm::mat4();
		static auto frame = 0;
		// update states, e.g. for animation
		static auto startTime = std::chrono::high_resolution_clock::now();

		mPostProcRenderer->start_frame();

		auto currentTime = std::chrono::high_resolution_clock::now();
		float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

		UniformBufferObject uboCam{};
		
#if TAA_ENABLED
		uboCam.frameOffset = (jitter[frame] - glm::vec2(0.5)) / glm::vec2(renderWidth / 2, renderHeight / 2);
#else
		uboCam.frameOffset = glm::vec2(0);
#endif

		uboCam.view = mCamera.view_matrix();
		uboCam.proj = mCamera.projection_matrix();

		float nearPlane = mCamera.near_plane_distance();
		float farPlane = mCamera.far_plane_distance();
		if (cgb::vulkan_context::instance().shadingRateImageSupported) {
			mCurrentVrsImageComputeDrawer->set_cam_data(uboCam, nearPlane, farPlane);
		}
		uboCam.proj = glm::translate(glm::vec3(uboCam.frameOffset.x * 2.0f, uboCam.frameOffset.y * 2.0f, 0.0f)) * uboCam.proj;

		// -----------------------------------

		// update point light position with view matrix 
		mDirLightBuffers[cgb::vulkan_context::instance().currentFrame]->update_buffer(&m_dir_light.GetGpuData(glm::mat3(uboCam.view)), sizeof(cgb::DirectionalLightGpuData));

		PointLights pointLights;
		for (int i = 0; i < MAX_COUNT_POINT_LIGHTS && i < mPointLights.size(); i++) {
			pointLights.pointLightData[i] = mPointLights[i].GetGpuData(uboCam.view);
		}
		pointLights.count = mPointLights.size();
		mPointLightsBuffers[cgb::vulkan_context::instance().currentFrame]->update_buffer(&pointLights, sizeof(pointLights));

		for (int i = 0; i < mSponzaModel->get_render_objects().size(); i++) {
			auto model = mSponzaModel->transformation_matrix() * mSponzaModel->mesh_at(i).transformation_matrix();
			auto sponzaRenderObject = mSponzaModel->get_render_objects()[i];
			uboCam.mv = uboCam.view * model;
			uboCam.mvp = uboCam.proj * uboCam.mv;
			uboCam.vmNormalMatrix = glm::mat4(glm::inverseTranspose(glm::mat3(uboCam.mv)));


			auto prevDynamicData = *mPrevFrameData[0].get();
			prevDynamicData.mvpMatrix *= model;
			sponzaRenderObject->update_uniform_buffer(cgb::vulkan_context::instance().currentFrame, uboCam);
			sponzaRenderObject->update_push_constant(std::make_shared<prev_frame_data>(prevDynamicData));
		}
		auto prevDynamicData = *mPrevFrameData[0].get();
		prevDynamicData.mvpMatrix *= prevFrameModel;
		for (int i = 0; i < mParallelPipesModel->get_render_objects().size(); i++) {
			auto model = mParallelPipesModel->transformation_matrix() * mParallelPipesModel->mesh_at(i).transformation_matrix();
			auto mParallelPipesObj = mParallelPipesModel->get_render_objects()[i];
			uboCam.mv = uboCam.view * model;
			uboCam.mvp = uboCam.proj * uboCam.mv;
			uboCam.vmNormalMatrix = glm::mat4(glm::inverseTranspose(glm::mat3(uboCam.mv)));
			prevFrameModel = model;

			mParallelPipesObj->update_uniform_buffer(cgb::vulkan_context::instance().currentFrame, uboCam);
			mParallelPipesObj->update_push_constant(std::make_shared<prev_frame_data>(prevDynamicData));
		}

		//auto oldFrameIdx = cgb::vulkan_context::instance().currentFrame;
		//cgb::vulkan_context::instance().currentFrame = 0;



		vk::CommandBufferInheritanceInfo inheritanceInfo = {};
		inheritanceInfo.occlusionQueryEnable = VK_FALSE;

		vk::CommandBufferBeginInfo beginInfo = {};
		beginInfo.flags = vk::CommandBufferUsageFlagBits::eSimultaneousUse;
		beginInfo.pInheritanceInfo = &inheritanceInfo;

		vk::ImageMemoryBarrier imgMemBarrier = {};
		if (cgb::vulkan_context::instance().shadingRateImageSupported) {
			imgMemBarrier.srcAccessMask = {};
			imgMemBarrier.dstAccessMask = vk::AccessFlagBits::eShaderWrite;
			imgMemBarrier.oldLayout = vk::ImageLayout::eUndefined;
			imgMemBarrier.newLayout = vk::ImageLayout::eGeneral;
			imgMemBarrier.image = vrsDebugImages[cgb::vulkan_context::instance().currentFrame]->get_image();
			imgMemBarrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
			imgMemBarrier.subresourceRange.baseArrayLayer = 0;
			imgMemBarrier.subresourceRange.layerCount = 1;
			imgMemBarrier.subresourceRange.levelCount = 1;

			vk::CommandBuffer commandBuffer = drawCommandBufferManager->get_command_buffer(vk::CommandBufferLevel::eSecondary, beginInfo);
			commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eFragmentShader, vk::PipelineStageFlagBits::eComputeShader, {}, nullptr, nullptr, imgMemBarrier);
		}
		// start drawing, record draw commands, etc.
		if (cgb::vulkan_context::instance().shadingRateImageSupported) {
			mVrsRenderer->render(std::vector<cgb::vulkan_render_object*>{}, mCurrentVrsImageComputeDrawer.get());
		}

		if (cgb::vulkan_context::instance().shadingRateImageSupported) {
			imgMemBarrier.srcAccessMask = vk::AccessFlagBits::eShaderWrite;
			imgMemBarrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;
			imgMemBarrier.oldLayout = vk::ImageLayout::eGeneral;
			imgMemBarrier.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;

			vk::CommandBuffer commandBuffer = drawCommandBufferManager->get_command_buffer(vk::CommandBufferLevel::eSecondary, beginInfo);
			commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eComputeShader, vk::PipelineStageFlagBits::eFragmentShader, {}, nullptr, nullptr, imgMemBarrier);
		}

		std::vector<cgb::vulkan_render_object*> renderObjects;
		for (auto sponzaRenderObject : mSponzaModel->get_render_objects()) {
			renderObjects.push_back(sponzaRenderObject.get());
		}
		for (auto parallelPipesRenderObject : mParallelPipesModel->get_render_objects()) {
			renderObjects.push_back(parallelPipesRenderObject.get());
		}
		//cgb::vulkan_context::instance().currentFrame = oldFrameIdx;

#if DEFERRED_SHADING
		mDefRend->draw(renderObjects);
#else
		mRenderer->render({ renderObjects }, mMaterialDrawer.get());		
#endif // DEFERRED_SHADING

		// TAA pass
		int tAAIndex = mTAAIndices[cgb::vulkan_context::instance().currentFrame];
		mTAAIndices[cgb::vulkan_context::instance().currentFrame] = (tAAIndex + 1) % 2; // switch index for the correct frame

		auto prevData = taa_prev_frame_data{};
		prevData.vPMatrix = mPrevFrameData[0]->mvpMatrix;
		prevData.invPMatrix = glm::inverse(mCamera.projection_matrix());
		prevData.invVMatrix = glm::inverse(uboCam.view);
		prevData.jitter = uboCam.frameOffset;
		mPostProcFullScreenQuads[tAAIndex]->update_push_constant(std::make_shared<taa_prev_frame_data>(prevData));

		mTAARenderer->set_framebuffer(mTAAFramebuffers[tAAIndex]);
		renderObjects.clear();
		renderObjects.push_back(mPostProcFullScreenQuads[tAAIndex].get());
		//mTAADrawer->set_vrs_images(vrsDefaultImage);
		//mPostProcDrawer->set_vrs_images(vrsDefaultImage);
		//mVrsDebugDrawer->set_vrs_images(vrsDefaultImage);
		//mMaterialDrawer->set_vrs_images(vrsDefaultImage);
		
		mTAAMeanVarianceRenderer->render(renderObjects, mTAAMeanVarianceDrawer.get());
		mTAAMeanVarianceRenderer->recordPrimaryCommandBuffer();
		// generate mipmaps for variance mean image
		inheritanceInfo = {};
		inheritanceInfo.occlusionQueryEnable = VK_FALSE;

		beginInfo = {};
		beginInfo.flags = vk::CommandBufferUsageFlagBits::eSimultaneousUse;
		beginInfo.pInheritanceInfo = &inheritanceInfo;
		vk::CommandBuffer commandBuffer = drawCommandBufferManager->get_command_buffer(vk::CommandBufferLevel::eSecondary, beginInfo);
		mTAAMeanVarianceImages[cgb::vulkan_context::instance().currentFrame]->generate_mipmaps(commandBuffer);
		mTAAVariance2Images[cgb::vulkan_context::instance().currentFrame]->generate_mipmaps(commandBuffer);
		mTAAMeanVarianceRenderer->add_recorded_secondary_command_buffers();

		mTAARenderer->render(renderObjects, mTAADrawer.get());

		if (cgb::vulkan_context::instance().shadingRateImageSupported) {
			renderObjects.clear();
			renderObjects.push_back(mVrsDebugFullscreenQuad.get());
			mTAARenderer->render(renderObjects, mVrsDebugDrawer.get());
		}

#if !BLIT_FINAL_IMAGE
		// post processing pass
		renderObjects.clear();
		renderObjects.push_back(mTAAFullScreenQuads[tAAIndex].get());
		mPostProcRenderer->render(renderObjects, mPostProcDrawer.get());
		
#else
		mTAARenderer->recordPrimaryCommandBuffer();
		cgb::vulkan_image::blit_image(mTAAImages[tAAIndex][cgb::vulkan_context::instance().currentFrame].get(),
			mImagePresenter->get_swap_chain_images()[cgb::vulkan_context::instance().currentSwapChainIndex].get(), vk::ImageLayout::eShaderReadOnlyOptimal,
			vk::ImageLayout::eShaderReadOnlyOptimal, vk::ImageLayout::eUndefined, vk::ImageLayout::ePresentSrcKHR,
			vk::AccessFlagBits::eColorAttachmentWrite, {}, {}, {},
			vk::PipelineStageFlagBits::eColorAttachmentOutput, vk::PipelineStageFlagBits::eBottomOfPipe, drawCommandBufferManager.get());
		mPostProcRenderer->add_recorded_secondary_command_buffers();
#endif

		mPostProcRenderer->end_frame();
		//cgb::vulkan_context::instance().device.waitIdle();

		//frame = (frame + int(0 == cgb::vulkan_context::instance().currentFrame)) % jitter.size();
		frame = (frame + 1) % jitter.size();
		mPrevFrameData[0]->imgSize = glm::vec2(renderWidth, renderHeight);
		mPrevFrameData[0]->mvpMatrix = mCamera.projection_matrix() * uboCam.view;

		//Sleep(280);
	}

	void createDescriptorSetLayout()
	{
		mResourceBundleLayout = std::make_shared<cgb::vulkan_resource_bundle_layout>();
		mResourceBundleLayout->add_binding(0, vk::DescriptorType::eUniformBuffer, cgb::ShaderStageFlagBits::eVertex);
		mResourceBundleLayout->add_binding(1, vk::DescriptorType::eCombinedImageSampler, cgb::ShaderStageFlagBits::eFragment);
		mResourceBundleLayout->add_binding(2, vk::DescriptorType::eCombinedImageSampler, cgb::ShaderStageFlagBits::eFragment);
		mResourceBundleLayout->bake();
	}

	void createVrsComputeDescriptorSetLayout()
	{
		vk::DescriptorSetLayoutBinding storageImageLayoutBinding = {};
		storageImageLayoutBinding.binding = 0;
		storageImageLayoutBinding.descriptorCount = 1;
		storageImageLayoutBinding.descriptorType = vk::DescriptorType::eStorageImage;
		storageImageLayoutBinding.pImmutableSamplers = nullptr;
		storageImageLayoutBinding.stageFlags = vk::ShaderStageFlagBits::eCompute;

		vk::DescriptorSetLayoutCreateInfo layoutInfo = {};
		std::array<vk::DescriptorSetLayoutBinding, 1> bindings = { storageImageLayoutBinding };
		layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
		layoutInfo.pBindings = bindings.data();


		if (cgb::vulkan_context::instance().device.createDescriptorSetLayout(&layoutInfo, nullptr, &vrsComputeDescriptorSetLayout) != vk::Result::eSuccess) {
			throw std::runtime_error("failed to create vrs compute descriptor set layout!");
		}
	}

	void createVrsComputeDescriptorPool()
	{
		std::array<vk::DescriptorPoolSize, 1> poolSizes = {};
		poolSizes[0].type = vk::DescriptorType::eStorageImage;
		poolSizes[0].descriptorCount = static_cast<uint32_t>(cgb::vulkan_context::instance().dynamicRessourceCount);

		vk::DescriptorPoolCreateInfo poolInfo = {};
		poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
		poolInfo.pPoolSizes = poolSizes.data();
		poolInfo.maxSets = static_cast<uint32_t>(cgb::vulkan_context::instance().dynamicRessourceCount);

		if (cgb::vulkan_context::instance().device.createDescriptorPool(&poolInfo, nullptr, &vrsComputeDescriptorPool) != vk::Result::eSuccess) {
			throw std::runtime_error("failed to create vrs compute descriptor pool!");
		}
	}

	void createVrsDescriptorSets()
	{
		std::vector<vk::DescriptorSetLayout> layouts(cgb::vulkan_context::instance().dynamicRessourceCount, vrsComputeDescriptorSetLayout);
		vk::DescriptorSetAllocateInfo allocInfo = {};
		allocInfo.descriptorPool = vrsComputeDescriptorPool;
		allocInfo.descriptorSetCount = static_cast<uint32_t>(cgb::vulkan_context::instance().dynamicRessourceCount);
		allocInfo.pSetLayouts = layouts.data();

		mVrsComputeDescriptorSets.resize(cgb::vulkan_context::instance().dynamicRessourceCount);
		if (cgb::vulkan_context::instance().device.allocateDescriptorSets(&allocInfo, mVrsComputeDescriptorSets.data()) != vk::Result::eSuccess) {
			throw std::runtime_error("failed to allocate descriptor sets!");
		}

		for (size_t i = 0; i < cgb::vulkan_context::instance().dynamicRessourceCount; i++) {
			vk::DescriptorImageInfo imageInfo = {};
			imageInfo.imageLayout = vk::ImageLayout::eGeneral;
			imageInfo.imageView = vrsImages[i]->get_image_view();

			std::array<vk::WriteDescriptorSet, 1> descriptorWrites = {};

			descriptorWrites[0].dstSet = mVrsComputeDescriptorSets[i];
			descriptorWrites[0].dstBinding = 0;
			descriptorWrites[0].dstArrayElement = 0;
			descriptorWrites[0].descriptorType = vk::DescriptorType::eStorageImage;
			descriptorWrites[0].descriptorCount = 1;
			descriptorWrites[0].pImageInfo = &imageInfo;

			cgb::vulkan_context::instance().device.updateDescriptorSets(static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
		}
	}

	// image / texture

	void createTexture()
	{
		int texWidth, texHeight, texChannels;
		stbi_uc* pixels = stbi_load(TEXTURE_PATH.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);

		if (!pixels) {
			throw std::runtime_error("failed to load texture image!");
		}
		textureImage = std::make_shared<cgb::vulkan_image>(pixels, texWidth, texHeight, texChannels);
		texture = std::make_shared<cgb::vulkan_texture>(textureImage);

		stbi_image_free(pixels);
	}

	// attachments for framebuffer (color image to render to before resolve, depth image)

	void createDepthResources()
	{
		vk::Format depthFormat = findDepthFormat();

		depthImage = std::make_shared<cgb::vulkan_image>(renderWidth, renderHeight, 1, 1, cgb::vulkan_context::instance().msaaSamples, depthFormat,
			vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eSampled, vk::MemoryPropertyFlagBits::eDeviceLocal, vk::ImageAspectFlagBits::eDepth);
		depthImage->transition_image_layout(depthFormat, vk::ImageLayout::eUndefined, vk::ImageLayout::eDepthStencilAttachmentOptimal, 1);
		cgb::vulkan_context::instance().defaultDepthImage = depthImage;
	}

	vk::Format findDepthFormat()
	{
		return findSupportedFormat(
			{ vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint },
			vk::ImageTiling::eOptimal,
			vk::FormatFeatureFlagBits::eDepthStencilAttachment
		);
	}

	vk::Format findSupportedFormat(const std::vector<vk::Format>& candidates, vk::ImageTiling tiling, vk::FormatFeatureFlags features)
	{
		for (vk::Format format : candidates) {
			vk::FormatProperties props;
			cgb::vulkan_context::instance().physicalDevice.getFormatProperties(format, &props);

			if (tiling == vk::ImageTiling::eLinear && (props.linearTilingFeatures & features) == features) {
				return format;
			}
			else if (tiling == vk::ImageTiling::eOptimal && (props.optimalTilingFeatures & features) == features) {
				return format;
			}
		}

		throw std::runtime_error("failed to find supported format!");
	}

	void createColorResources()
	{
		auto width = renderWidth;
		auto height = renderHeight;
		vk::Format colorFormat = mImagePresenter->get_swap_chain_image_format();
		vk::Format colorFormatMotionVectorImg = vk::Format::eR16G16Snorm;
		vk::Format colorFormatVrsPrevImg = vk::Format::eR16G16B16A16Unorm;

		colorImage = std::make_shared<cgb::vulkan_image>(width, height, 1, 4,
			cgb::vulkan_context::instance().msaaSamples, colorFormat, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eTransientAttachment | vk::ImageUsageFlagBits::eColorAttachment,
			vk::MemoryPropertyFlagBits::eDeviceLocal, vk::ImageAspectFlagBits::eColor);
		colorImage->transition_image_layout(colorFormat, vk::ImageLayout::eUndefined, vk::ImageLayout::eColorAttachmentOptimal, 1);

		mPostProcImages.resize(cgb::vulkan_context::instance().dynamicRessourceCount);
		mPostProcTextures.resize(cgb::vulkan_context::instance().dynamicRessourceCount);
		for (int i = 0; i < mTAAImages.size(); i++) {
			mTAAImages[i].resize(cgb::vulkan_context::instance().dynamicRessourceCount);
			mTAATextures[i].resize(cgb::vulkan_context::instance().dynamicRessourceCount);
		}

		for (int i = 0; i < cgb::vulkan_context::instance().dynamicRessourceCount; i++) {
			mPostProcImages[i] = std::make_shared<cgb::vulkan_image>(width, height, 1, 4, vk::SampleCountFlagBits::e1, colorFormat,
				vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferSrc,
				vk::MemoryPropertyFlagBits::eDeviceLocal, vk::ImageAspectFlagBits::eColor);
			mPostProcImages[i]->transition_image_layout(colorFormat, vk::ImageLayout::eUndefined, vk::ImageLayout::eShaderReadOnlyOptimal, 1);
			mPostProcTextures[i] = std::make_shared<cgb::vulkan_texture>(mPostProcImages[i]);

			for (int j = 0; j < mTAAImages.size(); j++) {
				mTAAImages[j][i] = std::make_shared<cgb::vulkan_image>(width, height, 1, 4, vk::SampleCountFlagBits::e1, colorFormat,
					vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferSrc,
					vk::MemoryPropertyFlagBits::eDeviceLocal, vk::ImageAspectFlagBits::eColor);
				if (j == 0) {
					mTAAImages[j][i]->transition_image_layout(colorFormat, vk::ImageLayout::eUndefined, vk::ImageLayout::eColorAttachmentOptimal, 1);
				}
				else {
					mTAAImages[j][i]->transition_image_layout(colorFormat, vk::ImageLayout::eUndefined, vk::ImageLayout::eShaderReadOnlyOptimal, 1);
				}
				mTAATextures[j][i] = std::make_shared<cgb::vulkan_texture>(mTAAImages[j][i]);
			}
		}
		mTAAImages[0][1] = mTAAImages[0][0];
		mTAAImages[0][2] = mTAAImages[0][0];
		mTAAImages[1][1] = mTAAImages[1][0];
		mTAAImages[1][2] = mTAAImages[1][0];
		mTAATextures[0][1] = mTAATextures[0][0];
		mTAATextures[0][2] = mTAATextures[0][0];
		mTAATextures[1][1] = mTAATextures[1][0];
		mTAATextures[1][2] = mTAATextures[1][0];

		// motion vector image
		//auto formatProperties = cgb::vulkan_context::instance().physicalDevice.getImageFormatProperties(
		//	colorFormatMotionVectorImg, vk::ImageType::e2D, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eColorAttachment, {});

		mMotionVectorMsaaImage = std::make_shared<cgb::vulkan_image>(width, height, 1, 2, cgb::vulkan_context::instance().msaaSamples, colorFormatMotionVectorImg,
			vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eColorAttachment,
			vk::MemoryPropertyFlagBits::eDeviceLocal, vk::ImageAspectFlagBits::eColor);
		mMotionVectorImages.resize(cgb::vulkan_context::instance().dynamicRessourceCount);
		mMotionVectorTextures.resize(cgb::vulkan_context::instance().dynamicRessourceCount);

		for (int i = 0; i < cgb::vulkan_context::instance().dynamicRessourceCount; i++) {
			mMotionVectorImages[i] = std::make_shared<cgb::vulkan_image>(width, height, 1, 2, vk::SampleCountFlagBits::e1, colorFormatMotionVectorImg,
				vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferSrc,
				vk::MemoryPropertyFlagBits::eDeviceLocal, vk::ImageAspectFlagBits::eColor);
			mMotionVectorImages[i]->transition_image_layout(colorFormatMotionVectorImg, vk::ImageLayout::eUndefined, vk::ImageLayout::eColorAttachmentOptimal, 1);
			mMotionVectorTextures[i] = std::make_shared<cgb::vulkan_texture>(mMotionVectorImages[i]);
		}

		mVrsPrevRenderImages.resize(cgb::vulkan_context::instance().dynamicRessourceCount);
		mVrsPrevRenderTextures.resize(cgb::vulkan_context::instance().dynamicRessourceCount);

		mVrsPrevRenderMsaaImage = std::make_shared<cgb::vulkan_image>(width, height, 1, 4, cgb::vulkan_context::instance().msaaSamples, colorFormatVrsPrevImg,
			vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eColorAttachment,
			vk::MemoryPropertyFlagBits::eDeviceLocal, vk::ImageAspectFlagBits::eColor);

		for (int i = 0; i < cgb::vulkan_context::instance().dynamicRessourceCount; i++) {
			mVrsPrevRenderImages[i] = std::make_shared<cgb::vulkan_image>(width, height, 1, 4, vk::SampleCountFlagBits::e1, colorFormatVrsPrevImg,
				vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferSrc,
				vk::MemoryPropertyFlagBits::eDeviceLocal, vk::ImageAspectFlagBits::eColor);
			mVrsPrevRenderImages[i]->transition_image_layout(colorFormatVrsPrevImg, vk::ImageLayout::eUndefined, vk::ImageLayout::eShaderReadOnlyOptimal, 1);
			mVrsPrevRenderTextures[i] = std::make_shared<cgb::vulkan_texture>(mVrsPrevRenderImages[i]);
		}
		//mVrsPrevRenderBlitImages[1] = mVrsPrevRenderBlitImages[0];
		//mVrsPrevRenderBlitImages[2] = mVrsPrevRenderBlitImages[0];
		//mVrsPrevRenderImages[1] = mVrsPrevRenderImages[0];
		//mVrsPrevRenderImages[2] = mVrsPrevRenderImages[0];

		mVrsEdgeImages.resize(cgb::vulkan_context::instance().dynamicRessourceCount);
		for (int i = 0; i < cgb::vulkan_context::instance().dynamicRessourceCount; i++) {
			mVrsEdgeImages[i] = std::make_shared<cgb::vulkan_image>(width, height, 1, 4, vk::SampleCountFlagBits::e1, colorFormat,
				vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferSrc,
				vk::MemoryPropertyFlagBits::eDeviceLocal, vk::ImageAspectFlagBits::eColor);
			mVrsEdgeImages[i]->transition_image_layout(colorFormatVrsPrevImg, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferSrcOptimal, 1);
		}
		mVrsEdgeImages[1] = mVrsEdgeImages[0];
		mVrsEdgeImages[2] = mVrsEdgeImages[0];


		mVrsTAAClipImages.resize(cgb::vulkan_context::instance().dynamicRessourceCount);

		for (int i = 0; i < cgb::vulkan_context::instance().dynamicRessourceCount; i++) {
			mVrsTAAClipImages[i] = std::make_shared<cgb::vulkan_image>(renderWidth, renderHeight, 1, 4, vk::SampleCountFlagBits::e1, colorFormat, vk::ImageTiling::eOptimal,
				vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eColorAttachment, vk::MemoryPropertyFlagBits::eDeviceLocal, vk::ImageAspectFlagBits::eColor);
			mVrsTAAClipImages[i]->transition_image_layout(colorFormat, vk::ImageLayout::eUndefined, vk::ImageLayout::eColorAttachmentOptimal, 1);
		}

		mVrsTAAClipImages[1] = mVrsTAAClipImages[0];
		mVrsTAAClipImages[2] = mVrsTAAClipImages[0];
	}

	void createVRSImageResources()
	{
		vk::Format colorFormat = vk::Format::eR8Uint;
		auto width = renderWidth / cgb::vulkan_context::instance().shadingRateImageProperties.shadingRateTexelSize.width;
		auto height = renderHeight / cgb::vulkan_context::instance().shadingRateImageProperties.shadingRateTexelSize.height;

		vk::Format colorFormatDebug = vk::Format::eR8G8B8A8Unorm;
		vk::Format colorFormatVrsPrevImg = vk::Format::eR16G16B16A16Unorm;
		vk::Format colorFormatMotionVectorImg = mMotionVectorImages[0]->get_format();
		vk::Format colorFormatSwapChain = mImagePresenter->get_swap_chain_image_format();


		vrsImages.resize(cgb::vulkan_context::instance().dynamicRessourceCount);
		vrsDefaultImage.resize(cgb::vulkan_context::instance().dynamicRessourceCount);

		vrsDebugImages.resize(cgb::vulkan_context::instance().dynamicRessourceCount);
		vrsDebugTextureImages.resize(cgb::vulkan_context::instance().dynamicRessourceCount);
		mVrsPrevRenderBlitImages.resize(cgb::vulkan_context::instance().dynamicRessourceCount);
		mVrsPrevRenderBlitTextures.resize(cgb::vulkan_context::instance().dynamicRessourceCount);
		mVrsMasMotionVecBlitImages.resize(cgb::vulkan_context::instance().dynamicRessourceCount);
		mVrsMasMotionVecBlitTextures.resize(cgb::vulkan_context::instance().dynamicRessourceCount);
		mVrsEdgeBlitImages.resize(cgb::vulkan_context::instance().dynamicRessourceCount);
		mVrsEdgeBlitTextures.resize(cgb::vulkan_context::instance().dynamicRessourceCount);

		std::vector clearPixels = std::vector<uint8_t>(size_t(width * height), 7);

		for (int i = 0; i < cgb::vulkan_context::instance().dynamicRessourceCount; i++) {
			vrsImages[i] = std::make_shared<cgb::vulkan_image>(width, height, 1, 1, vk::SampleCountFlagBits::e1, colorFormat, vk::ImageTiling::eOptimal,
				vk::ImageUsageFlagBits::eShadingRateImageNV | vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eDeviceLocal, vk::ImageAspectFlagBits::eColor);
			vrsImages[i]->transition_image_layout(colorFormat, vk::ImageLayout::eUndefined, vk::ImageLayout::eShadingRateOptimalNV, 1);

			vrsDefaultImage[i] = std::make_shared<cgb::vulkan_image>(width, height, 1, 1, vk::SampleCountFlagBits::e1, colorFormat, vk::ImageTiling::eOptimal,
				vk::ImageUsageFlagBits::eShadingRateImageNV | vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eTransferDst, vk::MemoryPropertyFlagBits::eDeviceLocal, vk::ImageAspectFlagBits::eColor);
			vrsDefaultImage[i]->update_pixels(clearPixels.data());
			vrsDefaultImage[i]->transition_image_layout(colorFormat, vk::ImageLayout::eUndefined, vk::ImageLayout::eShadingRateOptimalNV, 1);

			mVrsPrevRenderBlitImages[i] = std::make_shared<cgb::vulkan_image>(width, height, 1, 4, vk::SampleCountFlagBits::e1, colorFormatVrsPrevImg, vk::ImageTiling::eOptimal,
				vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eSampled, vk::MemoryPropertyFlagBits::eDeviceLocal, vk::ImageAspectFlagBits::eColor);
			mVrsPrevRenderBlitImages[i]->transition_image_layout(colorFormatVrsPrevImg, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal, 1);
			mVrsPrevRenderBlitTextures[i] = std::make_shared<cgb::vulkan_texture>(mVrsPrevRenderBlitImages[i]);

			mVrsEdgeBlitImages[i] = std::make_shared<cgb::vulkan_image>(width, height, 1, 4, vk::SampleCountFlagBits::e1, colorFormatSwapChain, vk::ImageTiling::eOptimal,
				vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled, vk::MemoryPropertyFlagBits::eDeviceLocal, vk::ImageAspectFlagBits::eColor);
			mVrsEdgeBlitImages[i]->transition_image_layout(colorFormatSwapChain, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal, 1);
			mVrsEdgeBlitTextures[i] = std::make_shared<cgb::vulkan_texture>(mVrsEdgeBlitImages[i]);

			mVrsMasMotionVecBlitImages[i] = std::make_shared<cgb::vulkan_image>(width, height, 1, 2, vk::SampleCountFlagBits::e1, colorFormatMotionVectorImg, vk::ImageTiling::eOptimal,
				vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eSampled, vk::MemoryPropertyFlagBits::eDeviceLocal, vk::ImageAspectFlagBits::eColor);
			mVrsMasMotionVecBlitImages[i]->transition_image_layout(colorFormatVrsPrevImg, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal, 1);
			mVrsMasMotionVecBlitTextures[i] = std::make_shared<cgb::vulkan_texture>(mVrsMasMotionVecBlitImages[i]);

			// Debug image
			vrsDebugImages[i] = std::make_shared<cgb::vulkan_image>(width, height, 1, 4, vk::SampleCountFlagBits::e1, colorFormatDebug, vk::ImageTiling::eOptimal,
				vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eSampled, vk::MemoryPropertyFlagBits::eDeviceLocal, vk::ImageAspectFlagBits::eColor);
			vrsDebugImages[i]->transition_image_layout(colorFormatDebug, vk::ImageLayout::eUndefined, vk::ImageLayout::eGeneral, 1);

			vrsDebugTextureImages[i] = std::make_shared<cgb::vulkan_texture>(vrsDebugImages[i]);
		}

		mVrsEdgeBlitImages[1] = mVrsEdgeBlitImages[0];
		mVrsEdgeBlitImages[2] = mVrsEdgeBlitImages[0];
		mVrsEdgeBlitTextures[1] = mVrsEdgeBlitTextures[0];
		mVrsEdgeBlitTextures[2] = mVrsEdgeBlitTextures[0];
	}

	void createVRSTAAImageResources()
	{
		vk::Format colorFormatSwapChain = mImagePresenter->get_swap_chain_image_format();
		auto width = renderWidth / cgb::vulkan_context::instance().shadingRateImageProperties.shadingRateTexelSize.width;
		auto height = renderHeight / cgb::vulkan_context::instance().shadingRateImageProperties.shadingRateTexelSize.height;

		mVrsTAAClipBlitImages.resize(cgb::vulkan_context::instance().dynamicRessourceCount);
		mVrsTAAClipBlitTextures.resize(cgb::vulkan_context::instance().dynamicRessourceCount);

		for (int i = 0; i < cgb::vulkan_context::instance().dynamicRessourceCount; i++) {
			mVrsTAAClipBlitImages[i] = std::make_shared<cgb::vulkan_image>(width, height, 1, 4, vk::SampleCountFlagBits::e1, colorFormatSwapChain, vk::ImageTiling::eOptimal,
				vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled, vk::MemoryPropertyFlagBits::eDeviceLocal, vk::ImageAspectFlagBits::eColor);
			mVrsTAAClipBlitImages[i]->transition_image_layout(colorFormatSwapChain, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal, 1);
			mVrsTAAClipBlitTextures[i] = std::make_shared<cgb::vulkan_texture>(mVrsTAAClipBlitImages[i]);
		}

		mVrsTAAClipBlitImages[1] = mVrsTAAClipBlitImages[0];
		mVrsTAAClipBlitImages[2] = mVrsTAAClipBlitImages[0];
		mVrsTAAClipBlitTextures[1] = mVrsTAAClipBlitTextures[0];
		mVrsTAAClipBlitTextures[2] = mVrsTAAClipBlitTextures[0];
	}

	void create_post_process_objects(vk::Viewport viewport, vk::Rect2D scissor) {
		mPostProcFramebuffer = std::make_shared<cgb::vulkan_framebuffer>(
			mImagePresenter->get_swap_chain_extent().width, mImagePresenter->get_swap_chain_extent().height,
			mImagePresenter->get_swap_chain_images_count(), vk::SampleCountFlagBits::e1);
		mPostProcFramebuffer->add_swapchain_color_attachment(mImagePresenter);
		//mPostProceFramebuffer->set_depth_attachment(depthImage);
		//mPostProceFramebuffer->add_dynamic_color_attachment(mVrsPrevRenderMsaaImage, mVrsPrevRenderImages, vk::ImageLayout::eTransferSrcOptimal);
		mPostProcFramebuffer->bake();

		mPostProcPipeline = std::make_shared<cgb::vulkan_pipeline>(mPostProcFramebuffer->get_render_pass(), viewport, scissor, vk::SampleCountFlagBits::e1,
			std::vector<std::shared_ptr<cgb::vulkan_resource_bundle_layout>> { mResourceBundleLayout }, sizeof(PushUniforms), cgb::ShaderStageFlagBits::eFragment);

		auto posAndUv = std::make_shared<cgb::vulkan_attribute_description_binding>(1, sizeof(Vertex), vk::VertexInputRate::eVertex);
		posAndUv->add_attribute_description(0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, pos));
		posAndUv->add_attribute_description(1, vk::Format::eR32G32Sfloat, offsetof(Vertex, texCoord));

		mPostProcPipeline->add_attr_desc_binding(posAndUv);
		mPostProcPipeline->add_shader(cgb::ShaderStageFlagBits::eVertex, "shaders/passthrough.vert.spv");
		mPostProcPipeline->add_shader(cgb::ShaderStageFlagBits::eFragment, "shaders/passthrough.frag.spv");
		mPostProcPipeline->disable_shading_rate_image();
		mPostProcPipeline->bake();

		mPostProcDrawer = std::make_unique<cgb::vulkan_drawer>(drawCommandBufferManager, mPostProcPipeline);
		mPostProcRenderer->set_framebuffer(mPostProcFramebuffer);
	}

	void create_TAA_objects(vk::Viewport viewport, vk::Rect2D scissor, std::shared_ptr<cgb::vulkan_resource_bundle_layout> layout) {
		mTAAIndices = std::vector<int>(cgb::vulkan_context::instance().dynamicRessourceCount, 0);

		// mean variance image
		auto width = renderWidth;
		auto height = renderHeight;
		auto mipLevels = cgb::vulkan_image::compute_mip_levels(width, height);
		vk::Format colorFormatMeanVarianceImg = vk::Format::eR16G16B16A16Snorm;
		vk::Format colorFormatVariance2Img = vk::Format::eR16G16Unorm;

		mTAAMeanVarianceImages.resize(cgb::vulkan_context::instance().dynamicRessourceCount);
		mTAAMeanVarianceTextures.resize(cgb::vulkan_context::instance().dynamicRessourceCount);
		mTAAVariance2Images.resize(cgb::vulkan_context::instance().dynamicRessourceCount);
		mTAAVariance2Textures.resize(cgb::vulkan_context::instance().dynamicRessourceCount);

		for (int i = 0; i < cgb::vulkan_context::instance().dynamicRessourceCount; i++) {
			mTAAMeanVarianceImages[i] = std::make_shared<cgb::vulkan_image>(width, height, mipLevels, 4, vk::SampleCountFlagBits::e1, colorFormatMeanVarianceImg,
				vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst,
				vk::MemoryPropertyFlagBits::eDeviceLocal, vk::ImageAspectFlagBits::eColor);
			mTAAMeanVarianceImages[i]->transition_image_layout(colorFormatMeanVarianceImg, vk::ImageLayout::eUndefined, vk::ImageLayout::eColorAttachmentOptimal, 1);
			mTAAMeanVarianceTextures[i] = std::make_shared<cgb::vulkan_texture>(mTAAMeanVarianceImages[i]);

			mTAAVariance2Images[i] = std::make_shared<cgb::vulkan_image>(width, height, mipLevels, 2, vk::SampleCountFlagBits::e1, colorFormatVariance2Img,
				vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst,
				vk::MemoryPropertyFlagBits::eDeviceLocal, vk::ImageAspectFlagBits::eColor);
			mTAAVariance2Images[i]->transition_image_layout(colorFormatMeanVarianceImg, vk::ImageLayout::eUndefined, vk::ImageLayout::eColorAttachmentOptimal, 1);
			mTAAVariance2Textures[i] = std::make_shared<cgb::vulkan_texture>(mTAAVariance2Images[i]);
		}

		mTAAMeanVarianceFramebuffer = std::make_shared<cgb::vulkan_framebuffer>(
			renderWidth, renderHeight,
			cgb::vulkan_context::instance().dynamicRessourceCount, vk::SampleCountFlagBits::e1);
		mTAAMeanVarianceFramebuffer->add_dynamic_color_attachment(mTAAMeanVarianceImages, vk::ImageLayout::eTransferDstOptimal);
		mTAAMeanVarianceFramebuffer->add_dynamic_color_attachment(mTAAVariance2Images, vk::ImageLayout::eTransferDstOptimal);
		mTAAMeanVarianceFramebuffer->add_dynamic_color_attachment(mVrsEdgeImages, vk::ImageLayout::eTransferSrcOptimal);
		mTAAMeanVarianceFramebuffer->bake();

		mTAAMeanVariancePipeline = std::make_shared<cgb::vulkan_pipeline>(mTAAMeanVarianceFramebuffer->get_render_pass(), viewport, scissor, vk::SampleCountFlagBits::e1, std::vector<std::shared_ptr<cgb::vulkan_resource_bundle_layout>> { layout }, sizeof(taa_prev_frame_data), cgb::ShaderStageFlagBits::eFragment);

		auto posAndUv = std::make_shared<cgb::vulkan_attribute_description_binding>(1, sizeof(Vertex), vk::VertexInputRate::eVertex);
		posAndUv->add_attribute_description(0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, pos));
		posAndUv->add_attribute_description(1, vk::Format::eR32G32Sfloat, offsetof(Vertex, texCoord));

		mTAAMeanVariancePipeline->add_attr_desc_binding(posAndUv);
		mTAAMeanVariancePipeline->add_shader(cgb::ShaderStageFlagBits::eVertex, "shaders/taa_mean_variance.vert.spv");
		mTAAMeanVariancePipeline->add_shader(cgb::ShaderStageFlagBits::eFragment, "shaders/taa_mean_variance.frag.spv");
		mTAAMeanVariancePipeline->add_color_blend_attachment_state(mTAAMeanVariancePipeline->get_color_blend_attachment_state(0));
		mTAAMeanVariancePipeline->add_color_blend_attachment_state(mTAAMeanVariancePipeline->get_color_blend_attachment_state(0));
		mTAAMeanVariancePipeline->disable_shading_rate_image();
		mTAAMeanVariancePipeline->bake();

		mTAAMeanVarianceDrawer = std::make_unique<cgb::vulkan_drawer>(drawCommandBufferManager, mTAAMeanVariancePipeline);
		//mTAADrawer->set_vrs_images(vrsDefaultImage);
		mTAAMeanVarianceRenderer->set_framebuffer(mTAAMeanVarianceFramebuffer);



		for (int i = 0; i < mTAAFramebuffers.size(); i++) {
			mTAAFramebuffers[i] = std::make_shared<cgb::vulkan_framebuffer>(
				renderWidth, renderHeight,
				cgb::vulkan_context::instance().dynamicRessourceCount, vk::SampleCountFlagBits::e1);
			mTAAFramebuffers[i]->add_dynamic_color_attachment(mTAAImages[i], vk::ImageLayout::eShaderReadOnlyOptimal);
			mTAAFramebuffers[i]->add_dynamic_color_attachment(mVrsTAAClipImages, vk::ImageLayout::eGeneral);
			mTAAFramebuffers[i]->bake();
		}

		// framebuffers are the same, therefore render passes are too and must be compatible! --> use render pass of first framebuffer
		mTAAPipeline = std::make_shared<cgb::vulkan_pipeline>(mTAAFramebuffers[0]->get_render_pass(), viewport, scissor, vk::SampleCountFlagBits::e1, std::vector<std::shared_ptr<cgb::vulkan_resource_bundle_layout>> { layout }, sizeof(taa_prev_frame_data), cgb::ShaderStageFlagBits::eFragment);

		mTAAPipeline->add_attr_desc_binding(posAndUv);
		mTAAPipeline->add_shader(cgb::ShaderStageFlagBits::eVertex, "shaders/taa.vert.spv");
		mTAAPipeline->add_shader(cgb::ShaderStageFlagBits::eFragment, "shaders/taa.frag.spv");
		mTAAPipeline->add_color_blend_attachment_state(mTAAMeanVariancePipeline->get_color_blend_attachment_state(0));
		mTAAPipeline->disable_shading_rate_image();
		mTAAPipeline->bake();

		mTAADrawer = std::make_unique<cgb::vulkan_drawer>(drawCommandBufferManager, mTAAPipeline);
		//mTAADrawer->set_vrs_images(vrsDefaultImage);
		mTAARenderer->set_framebuffer(mTAAFramebuffers[0]);

		jitter.resize(16);
		for (int i = 0; i < jitter.size(); i++) {
			jitter[i] = glm::vec2(halton(i, 2), halton(i, 3));
		}
	}

	double halton(int index, int base) {
		index++;
		double f = 1.0;
		double r = 0.0;

		while (index > 0) {
			f /= base;
			r = r + f * (index % base);
			index = glm::floor(index / base);
		}

		return r;
	}

	void load_models()
	{
		auto path = "assets/models/sponza/sponza_structure.obj";
		//auto path = "assets/models/island/island_final.dae";
		auto transform = glm::scale(glm::vec3(0.01f));
		auto  model_loader_flags = cgb::MOLF_triangulate | cgb::MOLF_smoothNormals | cgb::MOLF_calcTangentSpace;
		mSponzaModel = cgb::Model::LoadFromFile(path, transform, mResourceBundleGroup, model_loader_flags);
		mSponzaModel->create_render_objects(mMaterialObjectResourceBundleLayout);

		path = "assets/models/parallelepiped.obj";
		transform = glm::rotate(3.1415f / 2.0f, glm::vec3(0.0f, 1.0f, 0.0f)) * glm::scale(glm::vec3(0.7f));
		mParallelPipesModel = cgb::Model::LoadFromFile(path, transform, mResourceBundleGroup, model_loader_flags);

		if (mParallelPipesModel)
		{
			// If the parallelepiped has been loaded, just assign it sponza's "floor" material (I'm sure, we can find it)
			for (cgb::Mesh& mesh : mSponzaModel->SelectAllMeshes())
			{
				if (mesh.m_material_data->name().find("floor") != std::string::npos)
				{
					mParallelPipesModel->mesh_at(0).m_material_data = mesh.m_material_data;
					//mParallelPipesModel->mesh_at(0).mMaterialResourceBundle = mesh.m_material_data->create_resource_bundle(mResourceBundleGroup);
					mParallelPipesModel->mesh_at(0).mMaterialResourceBundle = mesh.mMaterialResourceBundle;
				}
			}
		}

		mParallelPipesModel->create_render_objects(mMaterialObjectResourceBundleLayout);
	}

	/*! Position of the first/second point light, or the rotation origin when animating its position. */
	glm::vec3 kInitialPositionOfFirstPointLight = glm::vec3(-0.64f, 0.45f, 3.35f);
	glm::vec3 kInitialPositionOfSecondPointLight = glm::vec3(-0.05f, 2.12f, 0.53f);

	void create_lots_of_lights()
	{
		mPointLights.clear();
		std::vector<glm::vec3> light_colors;
		light_colors.emplace_back(1.0f, 1.0f, 1.0f);
		light_colors.emplace_back(0.878f, 1.000f, 1.000f);
		light_colors.emplace_back(0.957f, 0.643f, 0.376f);
		light_colors.emplace_back(0.000f, 0.000f, 1.000f);
		light_colors.emplace_back(0.251f, 0.878f, 0.816f);
		light_colors.emplace_back(0.000f, 0.980f, 0.604f);
		light_colors.emplace_back(0.545f, 0.000f, 0.545f);
		light_colors.emplace_back(1.000f, 0.000f, 1.000f);
		light_colors.emplace_back(0.984f, 1.000f, 0.729f);
		light_colors.emplace_back(0.780f, 0.082f, 0.522f);
		light_colors.emplace_back(1.000f, 0.843f, 0.000f);
		light_colors.emplace_back(0.863f, 0.078f, 0.235f);
		light_colors.emplace_back(0.902f, 0.902f, 0.980f);
		light_colors.emplace_back(0.678f, 1.000f, 0.184f);

		std::default_random_engine generator;
		generator.seed(186);
		// generates numbers in the range 0..light_colors.size()-1
		std::uniform_int_distribution<size_t> distribution(0, light_colors.size() - 1);

		// Create a (moving) light near the pillars at the initial view
		mPointLights.push_back(cgb::PointLight(
			light_colors[distribution(generator)],
			kInitialPositionOfFirstPointLight,
			glm::vec4(1.0f, 0.0f, 5.0f, 0.0f)));

		// Create a (moving) light which is especially useful for evaluating Bonus-Task 1
		mPointLights.push_back(cgb::PointLight(
			light_colors[distribution(generator)],
			kInitialPositionOfSecondPointLight,
			glm::vec4(1.0f, 0.0f, 5.0f, 0.0f)));

		{ // Create lots of small lights near the floor
			const auto lb_x = -14.2f;
			const auto lb_z = -6.37f;
			const auto nx = 13;
			const auto nz = 6;
			const auto step_x = (12.93f - lb_x) / (nx - 1);
			const auto step_z = (5.65f - lb_z) / (nz - 1);
			const auto atten = glm::vec4(1.0f, 0.0f, 0.0f, 13.27f);
			for (auto x = 0; x < nx; ++x)
			{
				for (auto z = 0; z < nz; ++z)
				{
					mPointLights.push_back(cgb::PointLight(
						light_colors[distribution(generator)],
						glm::vec3(lb_x + x * step_x, 0.1f, lb_z + z * step_z),
						atten));
				}
			}
		}

		{	// Create several larger lights in the upper floor
			const auto lb_x = -13.36f;
			const auto lb_z = -5.46f;
			const auto nx = 6;
			const auto nz = 3;
			const auto step_x = (12.1f - lb_x) / (nx - 1);
			const auto step_z = (4.84f - lb_z) / (nz - 1);
			const auto atten = glm::vec4(1.0f, 0.0f, 5.666f, 0.0f);
			for (auto x = 0; x < nx; ++x)
			{
				for (auto z = 0; z < nz; ++z)
				{
					mPointLights.push_back(cgb::PointLight(
						light_colors[distribution(generator)],
						glm::vec3(lb_x + x * step_x, 7.0f, lb_z + z * step_z),
						atten));
				}
			}
		}
	}



	void AnimateFirstTwoPointLights(float elapsed_time)
	{
		const auto kSpeedXZ = 0.5f;
		const auto kRadiusXZ = 1.5f;
		mPointLights[0].set_position(kInitialPositionOfFirstPointLight + glm::vec3(
			kRadiusXZ * glm::sin(kSpeedXZ * elapsed_time),
			0.0f,
			kRadiusXZ * glm::cos(kSpeedXZ * elapsed_time)));

		const auto kSpeed = 0.6f;
		const auto kDistanceX = -0.23f;
		const auto kDistanceY = 1.0f;
		mPointLights[1].set_position(kInitialPositionOfSecondPointLight + glm::vec3(
			kDistanceX * glm::sin(kSpeed * elapsed_time),
			kDistanceY * glm::sin(kSpeed * elapsed_time),
			0.0f));
	}

	void start_capture_frame_data() {
		captureFrameTime = true;
		captureFrameTimeReset = true;
		frameTimeDataVector.clear();
	}

	void end_capture_frame_data(std::string filePostifx = "default", std::string title = "title") {
		captureFrameTime = false;

		std::ofstream outFile;
		outFile.open("frame_times_" + filePostifx, std::ios_base::app | std::ios_base::out);
		outFile << title;
		for (frame_time_data ftd : frameTimeDataVector) {
			outFile << ftd.sum_t / ftd.frame_count << "\t" << ftd.sum_t << "\t" << ftd.frame_count << "\n";
		}
		outFile.close();

		frameTimeDataVector.clear();
	}
};

int main()
{
	try {
		auto selectImageFormat = cgb::context_specific_function<cgb::image_format()>{}
		.SET_VULKAN_FUNCTION([]() { return cgb::image_format(vk::Format::eR8G8B8Unorm); })
			.SET_OPENGL46_FUNCTION([]() { return cgb::image_format{ GL_RGB };  });

		cgb::settings::gApplicationName = "Hello VRS";
		cgb::settings::gApplicationVersion = cgb::make_version(1, 0, 0);
		cgb::settings::gRequiredInstanceExtensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
		cgb::settings::gRequiredDeviceExtensions.push_back(VK_NV_SHADING_RATE_IMAGE_EXTENSION_NAME);

		// Create a window which we're going to use to render to
		auto mainWnd = cgb::context().create_window("Hello VRS!");
		mainWnd->set_resolution({ 1920, 1080 });
		mainWnd->set_presentaton_mode(cgb::presentation_mode::vsync);
		mainWnd->open();
		//mainWnd->switch_to_fullscreen_mode();

		// Create a "behavior" which contains functionality of our program
		auto vrsBehavior = vrs_behavior();

		// Create a composition of all things that define the essence of 
		// our program, which there are:
		//  - a timer
		//  - an executor
		//  - a window
		//  - a behavior
		auto hello = cgb::composition<cgb::varying_update_timer, cgb::sequential_executor>({
					&vrsBehavior,
					&(vrsBehavior.mCameraPath)
			});

		// Let's go:
		hello.start();
	}
	catch (std::runtime_error & re) {
		LOG_ERROR_EM(re.what());
	}
}


