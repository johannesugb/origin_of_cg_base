// hello_vrs.cpp : Defines the entry point for the console application.
//
#include <tobii/tobii.h>
#include <tobii/tobii_streams.h>
#include <random>

#include <vector>
#include <array>

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

#include "vrs_image_compute_drawer.h"
#include "vrs_cas_compute_drawer.h"
#include "vrs_mas_compute_drawer.h"

#include "eyetracking_interface.h"

#include "model.h"
#include "AmbientLight.h"
#include "DirectionalLight.h"
#include "PointLight.h"

#define VRS_EYE 0
#define VRS_CAS 0
#define VRS_MAS 1

const int WIDTH = 1920;
const int HEIGHT = 1080;


const int MAX_COUNT_POINT_LIGHTS = 100;

const std::string TEXTURE_PATH = "assets/chalet.jpg";

class vrs_behavior : public cgb::cg_element
{

public:
	vrs_behavior() : m_ambient_light(glm::vec3(1 / 255.0f, 2 / 255.0f, 3 / 255.0f)), m_dir_light(glm::vec3(13 / 255.0f, 17 / 255.0f, 27 / 255.0f), glm::vec3(-0.38f, -0.78f, -0.49f)) {
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
	std::unique_ptr<cgb::vrs_image_compute_drawer> mVrsImageComputeDrawer;

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
	std::shared_ptr<cgb::vulkan_pipeline> mRenderVulkanPipeline;
	std::shared_ptr<cgb::vulkan_pipeline> mComputeVulkanPipeline;
	std::shared_ptr<cgb::vulkan_framebuffer> mVulkanFramebuffer;
	std::shared_ptr<cgb::vulkan_resource_bundle_layout> mResourceBundleLayout;
	std::shared_ptr<cgb::vulkan_resource_bundle_group> mResourceBundleGroup;

	std::shared_ptr<eyetracking_interface> eyeInf;

	std::unique_ptr<cgb::Model> mModel;

	std::vector<std::shared_ptr<cgb::vulkan_render_object>> mSponzaRenderObjects;
	std::shared_ptr<cgb::vulkan_resource_bundle_layout> mGlobalResourceBundleLayout; // contains lights and global flags
	std::shared_ptr<cgb::vulkan_resource_bundle> mGlobalResourceBundle;

	std::shared_ptr<cgb::vulkan_resource_bundle_layout> mMaterialResourceBundleLayout; // contains material properties staying same for all objects
	std::shared_ptr<cgb::vulkan_resource_bundle_layout> mMaterialObjectResourceBundleLayout; // contains material properties varying for each object

	cgb::quake_camera mCamera;

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

	std::unique_ptr<vrs_cas_compute_drawer> mVrsCasComputeDrawer;
	std::shared_ptr<cgb::vulkan_pipeline> mVrsCasComputePipeline;

	// VRS: motion adaptive shading
	std::unique_ptr<vrs_mas_compute_drawer> mVrsMasComputeDrawer;
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

	std::vector<glm::vec2> jitter;


	std::shared_ptr<cgb::vulkan_image> mMotionVectorMsaaImage;
	std::vector<std::shared_ptr<cgb::vulkan_image>> mMotionVectorImages;
	std::vector<std::shared_ptr<cgb::vulkan_texture>> mMotionVectorTextures;

	struct prev_frame_data {
		glm::mat4 vPMatrix;
		glm::vec2 imgSize;
	};


public:
	void initialize() override
	{
		eyeInf = std::make_shared<eyetracking_interface>();
		//initWindow();
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
		if (cgb::input().key_pressed(cgb::key_code::f5)) {
			cgb::vulkan_context::instance().device.waitIdle();
			mVrsDebugPipeline->bake();
			mMaterialPipeline->bake();
			mRenderVulkanPipeline->bake();

			mComputeVulkanPipeline->bake();
			mVrsCasComputePipeline->bake();
			mVrsMasComputePipeline->bake();

			mPostProcPipeline->bake();
			mTAAPipeline->bake();
		}
		if (cgb::input().key_pressed(cgb::key_code::tab)) {
			if (mCamera.is_enabled()) {
				mCamera.disable();
			}
			else {
				mCamera.enable();
			}
		}
	}

	void render() override
	{
		static float sum_t = 0.0f;

		auto eyeData = eyeInf->get_eyetracking_data();

		drawFrame();

		sum_t += cgb::time().delta_time();
		if (sum_t >= 1.0f) {
			cgb::context().main_window()->set_title(std::to_string(1.0f / cgb::time().delta_time()).c_str());
			sum_t -= 1.0f;

			printf("Gaze point: %f, %f\n",
				   eyeData.positionX,
				   eyeData.positionY);
		}
	}

private:
	void initVulkan()
	{
		cgb::vulkan_context::instance().initVulkan();

		cgb::vulkan_context::instance().msaaSamples = vk::SampleCountFlagBits::e1;

		createCommandPools();

		transferCommandBufferManager = std::make_shared<cgb::vulkan_command_buffer_manager>(transferCommandPool, cgb::vulkan_context::instance().graphicsQueue);
		cgb::vulkan_context::instance().transferCommandBufferManager = transferCommandBufferManager;


		mImagePresenter = std::make_shared<cgb::vulkan_image_presenter>(cgb::vulkan_context::instance().presentQueue, cgb::vulkan_context::instance().surface, cgb::vulkan_context::instance().findQueueFamilies());
		cgb::vulkan_context::instance().dynamicRessourceCount = mImagePresenter->get_swap_chain_images_count();
		drawCommandBufferManager = std::make_shared<cgb::vulkan_command_buffer_manager>(mImagePresenter->get_swap_chain_images_count(), commandPool, cgb::vulkan_context::instance().graphicsQueue);
		mVulkanRenderQueue = std::make_shared<cgb::vulkan_render_queue>(cgb::vulkan_context::instance().graphicsQueue);

		std::vector<std::shared_ptr<cgb::vulkan_renderer>> dependentRenderers = {};
		if (cgb::vulkan_context::instance().shadingRateImageSupported) {
			mVrsRenderer = std::make_shared<cgb::vulkan_renderer>(nullptr, mVulkanRenderQueue, drawCommandBufferManager, std::vector<std::shared_ptr<cgb::vulkan_renderer>>{}, true);
			dependentRenderers.push_back(mVrsRenderer);
		}
		mRenderer = std::make_shared<cgb::vulkan_renderer>(nullptr, mVulkanRenderQueue, drawCommandBufferManager, dependentRenderers);
		mTAARenderer = std::make_shared<cgb::vulkan_renderer>(mImagePresenter, mVulkanRenderQueue, drawCommandBufferManager, std::vector<std::shared_ptr<cgb::vulkan_renderer>> { mRenderer });
		mPostProcRenderer = std::make_shared<cgb::vulkan_renderer>(mImagePresenter, mVulkanRenderQueue, drawCommandBufferManager, std::vector<std::shared_ptr<cgb::vulkan_renderer>> { mTAARenderer });


		createColorResources();
		createDepthResources();
		if (cgb::vulkan_context::instance().shadingRateImageSupported) {
			createVRSImageResources();
		}

		mVulkanFramebuffer = std::make_shared<cgb::vulkan_framebuffer>(mImagePresenter->get_swap_chain_extent().width, mImagePresenter->get_swap_chain_extent().height,
			mImagePresenter->get_swap_chain_images_count(), cgb::vulkan_context::instance().msaaSamples);
		mVulkanFramebuffer->add_dynamic_color_attachment(colorImage, mPostProcImages, vk::ImageLayout::eShaderReadOnlyOptimal);
		mVulkanFramebuffer->set_depth_attachment(depthImage);
		mVulkanFramebuffer->add_dynamic_color_attachment(mVrsPrevRenderMsaaImage, mVrsPrevRenderImages, vk::ImageLayout::eTransferSrcOptimal);
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
		viewport.width = (float)mImagePresenter->get_swap_chain_extent().width;
		viewport.height = (float)mImagePresenter->get_swap_chain_extent().height;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		vk::Rect2D scissor = {};
		scissor.offset = { 0, 0 };
		scissor.extent = mImagePresenter->get_swap_chain_extent();

		//Atribute description bindings
		auto bind1 = std::make_shared<vulkan_attribute_description_binding>(1, sizeof(Vertex), vk::VertexInputRate::eVertex);
		bind1->add_attribute_description(0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, pos));
		bind1->add_attribute_description(1, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, color));
		auto bind2 = std::make_shared<vulkan_attribute_description_binding>(2, sizeof(Vertex), vk::VertexInputRate::eVertex);
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
		vrsCasResourceBundleLayout->bake();
		auto vrsCasResourceBundle = mResourceBundleGroup->create_resource_bundle(vrsCasResourceBundleLayout, true);
		vrsCasResourceBundle->add_dynamic_image_resource(0, vk::ImageLayout::eGeneral, vrsImages);
		//vrsCasResourceBundle->add_dynamic_image_resource(1, vk::ImageLayout::eGeneral, mVrsPrevRenderBlitImages);
		vrsCasResourceBundle->add_dynamic_image_resource(1, vk::ImageLayout::eGeneral, mVrsPrevRenderBlitTextures);

		auto vrsMasResourceBundleLayout = std::make_shared<cgb::vulkan_resource_bundle_layout>();
		vrsMasResourceBundleLayout->add_binding(0, vk::DescriptorType::eStorageImage, cgb::ShaderStageFlagBits::eCompute);
		vrsMasResourceBundleLayout->add_binding(1, vk::DescriptorType::eCombinedImageSampler, cgb::ShaderStageFlagBits::eCompute);
		vrsMasResourceBundleLayout->bake();
		auto vrsMasResourceBundle = mResourceBundleGroup->create_resource_bundle(vrsMasResourceBundleLayout, true);
		vrsMasResourceBundle->add_dynamic_image_resource(0, vk::ImageLayout::eGeneral, vrsImages);
		vrsMasResourceBundle->add_dynamic_image_resource(1, vk::ImageLayout::eGeneral, mVrsMasMotionVecBlitTextures);

		auto vrsDebugResourceBundleLayout = std::make_shared<cgb::vulkan_resource_bundle_layout>();
		vrsDebugResourceBundleLayout->add_binding(0, vk::DescriptorType::eStorageImage, cgb::ShaderStageFlagBits::eCompute);
		vrsDebugResourceBundleLayout->bake();
		auto vrsDebugResourceBundle = mResourceBundleGroup->create_resource_bundle(vrsDebugResourceBundleLayout, true);
		vrsDebugResourceBundle->add_dynamic_image_resource(0, vk::ImageLayout::eGeneral, vrsDebugImages);

		if (cgb::vulkan_context::instance().shadingRateImageSupported) {
			drawer->set_vrs_images(vrsImages);

			// Compute Drawer and Pipeline
			mComputeVulkanPipeline = std::make_shared<cgb::vulkan_pipeline>(std::vector<std::shared_ptr<cgb::vulkan_resource_bundle_layout>> {}, sizeof(vrs_eye_comp_data));
			mComputeVulkanPipeline->add_shader(cgb::ShaderStageFlagBits::eCompute, "shaders/vrs_img.comp.spv");
			mComputeVulkanPipeline->add_resource_bundle_layout(std::make_shared<cgb::vulkan_resource_bundle_layout>(vrsComputeDescriptorSetLayout));

			mVrsImageComputeDrawer = std::make_unique<cgb::vrs_image_compute_drawer>(drawCommandBufferManager, mComputeVulkanPipeline, vrsDebugImages);
			mVrsImageComputeDrawer->set_vrs_images(vrsImages);
			mVrsImageComputeDrawer->set_descriptor_sets(mVrsComputeDescriptorSets);
			mVrsImageComputeDrawer->set_width_height(vrsImages[0]->get_width(), vrsImages[0]->get_height());
			mVrsImageComputeDrawer->set_eye_inf(eyeInf);

			// CAS
			mVrsCasComputePipeline = std::make_shared<cgb::vulkan_pipeline>(std::vector<std::shared_ptr<cgb::vulkan_resource_bundle_layout>> { vrsCasResourceBundleLayout, vrsDebugResourceBundleLayout }, sizeof(vrs_cas_comp_data));
			mVrsCasComputePipeline->add_shader(cgb::ShaderStageFlagBits::eCompute, "shaders/vrs_cas_img.comp.spv");
			mVrsCasComputePipeline->bake();

			mVrsCasComputeDrawer = std::make_unique<vrs_cas_compute_drawer>(drawCommandBufferManager, mVrsCasComputePipeline, std::vector<std::shared_ptr<cgb::vulkan_resource_bundle>> { vrsCasResourceBundle, vrsDebugResourceBundle },
				mVrsPrevRenderImages, mVrsPrevRenderBlitImages);
			mVrsCasComputeDrawer->set_vrs_images(vrsImages);

			// MAS
			mVrsMasComputePipeline = std::make_shared<cgb::vulkan_pipeline>(std::vector<std::shared_ptr<cgb::vulkan_resource_bundle_layout>> { vrsMasResourceBundleLayout, vrsDebugResourceBundleLayout }, sizeof(vrs_cas_comp_data));
			mVrsMasComputePipeline->add_shader(cgb::ShaderStageFlagBits::eCompute, "shaders/vrs_mas_img.comp.spv");
			mVrsMasComputePipeline->bake();

			mVrsMasComputeDrawer = std::make_unique<vrs_mas_compute_drawer>(drawCommandBufferManager, mVrsMasComputePipeline, std::vector<std::shared_ptr<cgb::vulkan_resource_bundle>> { vrsMasResourceBundle, vrsDebugResourceBundle },
				mMotionVectorImages, mVrsMasMotionVecBlitImages);
			mVrsMasComputeDrawer->set_vrs_images(vrsImages);

			createVrsComputeDescriptorPool();
			createVrsDescriptorSets();
			mVrsCasComputeDrawer->set_width_height(vrsImages[0]->get_width(), vrsImages[0]->get_height());
			mVrsMasComputeDrawer->set_width_height(vrsImages[0]->get_width(), vrsImages[0]->get_height());
		}
		mVrsDebugFullscreenQuad = std::make_shared<cgb::vulkan_render_object>(verticesScreenQuad, indicesScreenQuad, mResourceBundleLayout, mResourceBundleGroup, texture, vrsDebugTextureImages);

		// post process objects
		auto postProcResourceBundleLayout = std::make_shared<cgb::vulkan_resource_bundle_layout>();
		postProcResourceBundleLayout->add_binding(0, vk::DescriptorType::eCombinedImageSampler, cgb::ShaderStageFlagBits::eFragment);
		postProcResourceBundleLayout->add_binding(1, vk::DescriptorType::eCombinedImageSampler, cgb::ShaderStageFlagBits::eFragment);
		postProcResourceBundleLayout->add_binding(2, vk::DescriptorType::eCombinedImageSampler, cgb::ShaderStageFlagBits::eFragment);
		postProcResourceBundleLayout->bake();
		auto postProcBundles = std::array<std::shared_ptr<cgb::vulkan_resource_bundle>, 2>{};
		for (int i = 0; i < postProcBundles.size(); i++) {
			postProcBundles[i]  = mResourceBundleGroup->create_resource_bundle(postProcResourceBundleLayout, true);
		}
		postProcBundles[0]->add_dynamic_image_resource(0, vk::ImageLayout::eShaderReadOnlyOptimal, mPostProcTextures);
		postProcBundles[1]->add_dynamic_image_resource(0, vk::ImageLayout::eShaderReadOnlyOptimal, mPostProcTextures);
		// add previous frame image	
		postProcBundles[0]->add_dynamic_image_resource(1, vk::ImageLayout::eShaderReadOnlyOptimal, mTAATextures[1]);
		postProcBundles[1]->add_dynamic_image_resource(1, vk::ImageLayout::eShaderReadOnlyOptimal, mTAATextures[0]);

		postProcBundles[0]->add_dynamic_image_resource(2, vk::ImageLayout::eShaderReadOnlyOptimal, mMotionVectorTextures);
		postProcBundles[1]->add_dynamic_image_resource(2, vk::ImageLayout::eShaderReadOnlyOptimal, mMotionVectorTextures);


		for (int i = 0; i < mTAAFullScreenQuads.size(); i++) {
			mPostProcFullScreenQuads[i] = std::make_shared<cgb::vulkan_render_object>(verticesScreenQuad, indicesScreenQuad, std::vector<std::shared_ptr<cgb::vulkan_resource_bundle>> { postProcBundles[i] });
			mTAAFullScreenQuads[i] = std::make_shared<cgb::vulkan_render_object>(verticesScreenQuad, indicesScreenQuad, mResourceBundleLayout, mResourceBundleGroup, texture, mTAATextures[i]);
		}

		// Post processing
		create_post_process_objects(viewport, scissor);

		// TAA
		create_TAA_objects(viewport, scissor, postProcResourceBundleLayout);


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

		load_model("assets/models/sponza/sponza_structure.obj", glm::scale(glm::vec3(0.01f)), cgb::MOLF_triangulate | cgb::MOLF_smoothNormals | cgb::MOLF_calcTangentSpace, mModel);


		//Atribute description bindings
		auto& mesh = mModel->mesh_at(0);
		auto attrib_config = cgb::VertexAttribData::Nothing;
		attrib_config = attrib_config | cgb::VertexAttribData::Position;
		attrib_config = attrib_config | cgb::VertexAttribData::Tex2D;
		attrib_config = attrib_config | cgb::VertexAttribData::Normal;
		attrib_config = attrib_config | cgb::VertexAttribData::Tangents;
		attrib_config = attrib_config | cgb::VertexAttribData::Bitangents;

		auto sponzaBinding = std::shared_ptr<vulkan_attribute_description_binding>(new vulkan_attribute_description_binding(mesh.GetOrCreateForVertexAttribConfig(attrib_config)));

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
		//mQuakeCam.set_orthographic_projection(-5, 5, -5, 5, 0.5, 100);
		cgb::current_composition().add_element(mCamera);

		UniformBufferObject uboCam{};
		uboCam.model = mesh.transformation_matrix();
		uboCam.view = mCamera.view_matrix();
		uboCam.proj = mCamera.projection_matrix();
		uboCam.mv = uboCam.view * uboCam.model;

		for (auto sponzaRenderObject : mSponzaRenderObjects) {
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
		mVrsDebugPipeline = std::make_shared<cgb::vulkan_pipeline>(mPostProcFramebuffer->get_render_pass(), viewport, scissor, cgb::vulkan_context::instance().msaaSamples, std::vector<std::shared_ptr<cgb::vulkan_resource_bundle_layout>> { mResourceBundleLayout }, sizeof(PushUniforms));
		mVrsDebugPipeline->add_attr_desc_binding(bind1);
		mVrsDebugPipeline->add_shader(cgb::ShaderStageFlagBits::eVertex, "shaders/vrs_debug.vert.spv");
		mVrsDebugPipeline->add_shader(cgb::ShaderStageFlagBits::eFragment, "shaders/vrs_debug.frag.spv");

		auto& colorBlend = mVrsDebugPipeline->get_color_blend_attachment_state(0);
		colorBlend.blendEnable = VK_TRUE;
		colorBlend.dstColorBlendFactor = vk::BlendFactor::eOne;
		//mVrsDebugPipeline->add_color_blend_attachment_state(colorBlend);
		//mVrsDebugPipeline->disable_shading_rate_image();
		mVrsDebugPipeline->bake();
		mVrsDebugDrawer = std::make_unique<cgb::vulkan_drawer>(drawCommandBufferManager, mVrsDebugPipeline);
		if (cgb::vulkan_context::instance().shadingRateImageSupported) {
			mVrsDebugDrawer->set_vrs_images(vrsImages);
			mResourceBundleGroup->allocate_resource_bundle(vrsCasResourceBundle.get());
			mResourceBundleGroup->allocate_resource_bundle(vrsMasResourceBundle.get());
			mResourceBundleGroup->allocate_resource_bundle(vrsDebugResourceBundle.get());
		}
	}

	void cleanup()
	{
		cleanupSwapChain();

		mResourceBundleGroup.reset();
		cgb::vulkan_context::instance().device.destroyDescriptorPool(vrsComputeDescriptorPool);

		mVrsDebugFullscreenQuad.reset();
		for (auto sponzaRenderObject : mSponzaRenderObjects) {
			sponzaRenderObject.reset();
		}
		texture.reset();
		textureImage.reset();
		transferCommandBufferManager.reset();
		mVulkanRenderQueue.reset();
		drawCommandBufferManager.reset();

		cgb::vulkan_context::instance().device.destroyCommandPool(transferCommandPool);
		cgb::vulkan_context::instance().device.destroyCommandPool(commandPool);

		// destroy instance (in context)

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
			mVrsImageComputeDrawer.reset();
			mVrsCasComputeDrawer.reset();
			mVrsMasComputeDrawer.reset();
		}
		mMaterialDrawer.reset();
		mVrsDebugDrawer.reset();
		mRenderVulkanPipeline.reset();
		mComputeVulkanPipeline.reset();
		mVrsCasComputePipeline.reset();
		mVrsMasComputePipeline.reset();
		mVrsDebugPipeline.reset();
		mVulkanFramebuffer.reset();

		mPostProcDrawer.reset();
		mPostProcPipeline.reset();
		mPostProcFramebuffer.reset();

		mTAADrawer.reset();
		mTAAPipeline.reset();
		for (int i = 0; i < mTAAFramebuffers.size(); i++) {
			mTAAFramebuffers[i].reset();
		}

		mImagePresenter.reset();
		if (cgb::vulkan_context::instance().shadingRateImageSupported) {
			mVrsRenderer.reset();
		}
		mRenderer.reset();
		mTAARenderer.reset();
		mPostProcRenderer.reset();
	}

	//void recreateSwapChain()
	//{
	//	int width = 0, height = 0;
	//	while (width == 0 || height == 0) {
	//		glfwGetFramebufferSize(cgb::current_composition().window_in_focus()->handle()->mHandle, &width, &height);
	//		glfwWaitEvents();
	//	}

	//	cgb::vulkan_context::instance().device.waitIdle();

	//	cleanupSwapChain();

	//	imagePresenter = std::make_shared<vkImagePresenter>(cgb::vulkan_context::instance().presentQueue, cgb::vulkan_context::instance().surface, cgb::vulkan_context::instance().findQueueFamilies());
	//	mRenderer = std::make_unique<cgb::vulkan_renderer>(imagePresenter, mVulkanRenderQueue, drawCommandBufferManager);
	//	createColorResources();
	//	createDepthResources();
	//	mVulkanFramebuffer = std::make_shared<cgb::vulkan_framebuffer>(cgb::vulkan_context::instance().msaaSamples, colorImage, depthImage, imagePresenter);

	//	vk::Viewport viewport = {};
	//	viewport.x = 0.0f;
	//	viewport.y = 0.0f;
	//	viewport.width = (float)imagePresenter->get_swap_chain_extent().width;
	//	viewport.height = (float)imagePresenter->get_swap_chain_extent().height;
	//	viewport.minDepth = 0.0f;
	//	viewport.maxDepth = 1.0f;

	//	vk::Rect2D scissor = {};
	//	scissor.offset = { 0, 0 };
	//	scissor.extent = imagePresenter->get_swap_chain_extent();

	//	mRenderVulkanPipeline = std::make_shared<cgb::vulkan_pipeline>(mVulkanFramebuffer->get_render_pass(), viewport, scissor, cgb::vulkan_context::instance().msaaSamples, descriptorSetLayout);
	//	drawer = std::make_unique<vkDrawer>(drawCommandBufferManager, mRenderVulkanPipeline);
	//}

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
		static auto prevFrameData = std::make_shared<prev_frame_data>();
		static auto frame = 0;
		// update states, e.g. for animation
		static auto startTime = std::chrono::high_resolution_clock::now();

		mPostProcRenderer->start_frame();

		auto currentTime = std::chrono::high_resolution_clock::now();
		float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

		UniformBufferObject uboCam{};
		// -----> !ACHTUNG! Neu von JU <------
		uboCam.view = mCamera.view_matrix();
		uboCam.proj = mCamera.projection_matrix();
		//uboCam.proj = glm::translate(glm::vec3(jitter[frame]/glm::vec2(imagePresenter->get_swap_chain_extent().width/4, imagePresenter->get_swap_chain_extent().height/4), 0))
		//	* uboCam.proj;
		uboCam.frameOffset = (jitter[frame] - glm::vec2(0.5))/glm::vec2(mImagePresenter->get_swap_chain_extent().width/2, mImagePresenter->get_swap_chain_extent().height/2);
		// -----------------------------------

		// update point light position with view matrix 
		mDirLightBuffers[cgb::vulkan_context::instance().currentFrame]->update_buffer(&m_dir_light.GetGpuData(glm::mat3(uboCam.view)), sizeof(cgb::DirectionalLightGpuData));
		
		PointLights pointLights;
		for (int i = 0; i < MAX_COUNT_POINT_LIGHTS && i < mPointLights.size(); i++) {
			pointLights.pointLightData[i] = mPointLights[i].GetGpuData(uboCam.view);
		}
		pointLights.count = mPointLights.size();
		mPointLightsBuffers[cgb::vulkan_context::instance().currentFrame]->update_buffer(&pointLights, sizeof(pointLights));

		float nearPlane = mCamera.near_plane_distance();
		float farPlane = mCamera.far_plane_distance();
		mVrsCasComputeDrawer->set_cam_data(uboCam, nearPlane, farPlane);

		for (int i = 0; i < mSponzaRenderObjects.size(); i++) {
			auto sponzaRenderObject = mSponzaRenderObjects[i];
			uboCam.model = mModel->transformation_matrix();
			uboCam.model = glm::scale(glm::vec3(0.01f)) * mModel->mesh_at(i).transformation_matrix();
			//uboCam.model = mModel->transformation_matrix(i);
			uboCam.mv = uboCam.view * uboCam.model;
			uboCam.mvp = uboCam.proj * uboCam.view * uboCam.model;

			sponzaRenderObject->update_uniform_buffer(cgb::vulkan_context::instance().currentFrame, uboCam);
			sponzaRenderObject->update_push_constant(prevFrameData);
		}

		// start drawing, record draw commands, etc.
		if (cgb::vulkan_context::instance().shadingRateImageSupported) {
#if VRS_EYE
			mVrsRenderer->render(std::vector<cgb::vulkan_render_object*>{}, mVrsImageComputeDrawer.get());
#elif VRS_CAS
			mVrsRenderer->render(std::vector<cgb::vulkan_render_object*>{}, mVrsCasComputeDrawer.get());
#elif VRS_MAS
			mVrsRenderer->render(std::vector<cgb::vulkan_render_object*>{}, mVrsMasComputeDrawer.get());
#endif
		}

		std::vector<cgb::vulkan_render_object*> renderObjects;
		for (auto sponzaRenderObject : mSponzaRenderObjects) {
			renderObjects.push_back(sponzaRenderObject.get());
		}
		mRenderer->render({ renderObjects }, mMaterialDrawer.get());

		// TAA pass
		int tAAIndex = mTAAIndices[cgb::vulkan_context::instance().currentFrame];
		mTAAIndices[cgb::vulkan_context::instance().currentFrame] = (tAAIndex + 1) % 2; // switch index for the correct frame

		mPostProcFullScreenQuads[tAAIndex]->update_push_constant(std::make_shared<vrs_cas_comp_data>(mVrsCasComputeDrawer->get_push_constant_data()));

		mTAARenderer->set_framebuffer(mTAAFramebuffers[tAAIndex]);
		renderObjects.clear();
		renderObjects.push_back(mPostProcFullScreenQuads[tAAIndex].get());
		mTAADrawer->set_vrs_images(vrsDefaultImage);
		mPostProcDrawer->set_vrs_images(vrsDefaultImage);
		mVrsDebugDrawer->set_vrs_images(vrsDefaultImage);
		//mMaterialDrawer->set_vrs_images(vrsDefaultImage);
		mTAARenderer->render(renderObjects, mTAADrawer.get());

		// post processing pass
		renderObjects.clear();
		renderObjects.push_back(mTAAFullScreenQuads[tAAIndex].get());
		mPostProcRenderer->render(renderObjects, mPostProcDrawer.get());

		renderObjects.clear();
		renderObjects.push_back(mVrsDebugFullscreenQuad.get());
		mPostProcRenderer->render(renderObjects, mVrsDebugDrawer.get());

		mPostProcRenderer->end_frame();
		//cgb::vulkan_context::instance().device.waitIdle();

		frame = (frame + int(2 == cgb::vulkan_context::instance().currentFrame)) % jitter.size();
		prevFrameData->imgSize = glm::vec2(mImagePresenter->get_swap_chain_extent().width, mImagePresenter->get_swap_chain_extent().height);
		prevFrameData->vPMatrix = uboCam.proj * uboCam.view;
	}

	void createDescriptorSetLayout()
	{
		mResourceBundleLayout = std::make_shared<cgb::vulkan_resource_bundle_layout>();
		mResourceBundleLayout->add_binding(0, vk::DescriptorType::eUniformBuffer, cgb::ShaderStageFlagBits::eVertex);
		mResourceBundleLayout->add_binding(1, vk::DescriptorType::eCombinedImageSampler, cgb::ShaderStageFlagBits::eFragment);

		if (cgb::vulkan_context::instance().shadingRateImageSupported) {
			mResourceBundleLayout->add_binding(2, vk::DescriptorType::eCombinedImageSampler, cgb::ShaderStageFlagBits::eFragment);
		}
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
		poolSizes[0].descriptorCount = static_cast<uint32_t>(mImagePresenter->get_swap_chain_images_count());

		vk::DescriptorPoolCreateInfo poolInfo = {};
		poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
		poolInfo.pPoolSizes = poolSizes.data();
		poolInfo.maxSets = static_cast<uint32_t>(mImagePresenter->get_swap_chain_images_count());

		if (cgb::vulkan_context::instance().device.createDescriptorPool(&poolInfo, nullptr, &vrsComputeDescriptorPool) != vk::Result::eSuccess) {
			throw std::runtime_error("failed to create vrs compute descriptor pool!");
		}
	}

	void createVrsDescriptorSets()
	{
		std::vector<vk::DescriptorSetLayout> layouts(mImagePresenter->get_swap_chain_images_count(), vrsComputeDescriptorSetLayout);
		vk::DescriptorSetAllocateInfo allocInfo = {};
		allocInfo.descriptorPool = vrsComputeDescriptorPool;
		allocInfo.descriptorSetCount = static_cast<uint32_t>(mImagePresenter->get_swap_chain_images_count());
		allocInfo.pSetLayouts = layouts.data();

		mVrsComputeDescriptorSets.resize(mImagePresenter->get_swap_chain_images_count());
		if (cgb::vulkan_context::instance().device.allocateDescriptorSets(&allocInfo, mVrsComputeDescriptorSets.data()) != vk::Result::eSuccess) {
			throw std::runtime_error("failed to allocate descriptor sets!");
		}

		for (size_t i = 0; i < mImagePresenter->get_swap_chain_images_count(); i++) {
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

		depthImage = std::make_shared<cgb::vulkan_image>(mImagePresenter->get_swap_chain_extent().width, mImagePresenter->get_swap_chain_extent().height, 1, 1, cgb::vulkan_context::instance().msaaSamples, depthFormat,
												  vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eDepthStencilAttachment, vk::MemoryPropertyFlagBits::eDeviceLocal, vk::ImageAspectFlagBits::eDepth);
		depthImage->transition_image_layout(depthFormat, vk::ImageLayout::eUndefined, vk::ImageLayout::eDepthStencilAttachmentOptimal, 1);

	}

	vk::Format findDepthFormat()
	{
		return findSupportedFormat(
			{ vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint },
			vk::ImageTiling::eOptimal,
			vk::FormatFeatureFlagBits::eDepthStencilAttachment
		);
	}

	vk::Format findSupportedFormat(const std::vector<vk::Format> & candidates, vk::ImageTiling tiling, vk::FormatFeatureFlags features)
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
		auto width = mImagePresenter->get_swap_chain_extent().width;
		auto height = mImagePresenter->get_swap_chain_extent().height;
		vk::Format colorFormat = mImagePresenter->get_swap_chain_image_format();		
		vk::Format colorFormatMotionVectorImg = vk::Format::eR16G16Snorm;

		colorImage = std::make_shared<cgb::vulkan_image>(width, height, 1, 4,
			cgb::vulkan_context::instance().msaaSamples, colorFormat, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eTransientAttachment | vk::ImageUsageFlagBits::eColorAttachment, 
			vk::MemoryPropertyFlagBits::eDeviceLocal, vk::ImageAspectFlagBits::eColor);
		colorImage->transition_image_layout(colorFormat, vk::ImageLayout::eUndefined, vk::ImageLayout::eColorAttachmentOptimal, 1);

		mPostProcImages.resize(cgb::vulkan_context::instance().cgb::vulkan_context::instance().dynamicRessourceCount);
		mPostProcTextures.resize(cgb::vulkan_context::instance().cgb::vulkan_context::instance().dynamicRessourceCount);
		for (int i = 0; i < mTAAImages.size(); i++) {
			mTAAImages[i].resize(cgb::vulkan_context::instance().cgb::vulkan_context::instance().dynamicRessourceCount);
			mTAATextures[i].resize(cgb::vulkan_context::instance().cgb::vulkan_context::instance().dynamicRessourceCount);
		}

		for (int i = 0; i < cgb::vulkan_context::instance().cgb::vulkan_context::instance().dynamicRessourceCount; i++) {
			mPostProcImages[i] = std::make_shared<cgb::vulkan_image>(width, height, 1, 4, vk::SampleCountFlagBits::e1, colorFormat,
				vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eColorAttachment,
				vk::MemoryPropertyFlagBits::eDeviceLocal, vk::ImageAspectFlagBits::eColor);
			mPostProcImages[i]->transition_image_layout(colorFormat, vk::ImageLayout::eUndefined, vk::ImageLayout::eColorAttachmentOptimal, 1);
			mPostProcTextures[i] = std::make_shared<cgb::vulkan_texture>(mPostProcImages[i]);

			for (int j = 0; j < mTAAImages.size(); j++) {
				mTAAImages[j][i] = std::make_shared<cgb::vulkan_image>(width, height, 1, 4, vk::SampleCountFlagBits::e1, colorFormat,
					vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eColorAttachment,
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

		// motion vector image
		//auto formatProperties = cgb::vulkan_context::instance().physicalDevice.getImageFormatProperties(
		//	colorFormatMotionVectorImg, vk::ImageType::e2D, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eColorAttachment, {});

		mMotionVectorMsaaImage = std::make_shared<cgb::vulkan_image>(width, height, 1, 2, cgb::vulkan_context::instance().msaaSamples, colorFormatMotionVectorImg,
			vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eColorAttachment,
			vk::MemoryPropertyFlagBits::eDeviceLocal, vk::ImageAspectFlagBits::eColor);
		mMotionVectorImages.resize(cgb::vulkan_context::instance().cgb::vulkan_context::instance().dynamicRessourceCount);
		mMotionVectorTextures.resize(cgb::vulkan_context::instance().cgb::vulkan_context::instance().dynamicRessourceCount);

		for (int i = 0; i < cgb::vulkan_context::instance().cgb::vulkan_context::instance().dynamicRessourceCount; i++) {
			mMotionVectorImages[i] = std::make_shared<cgb::vulkan_image>(width, height, 1, 2, vk::SampleCountFlagBits::e1, colorFormatMotionVectorImg,
				vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferSrc,
				vk::MemoryPropertyFlagBits::eDeviceLocal, vk::ImageAspectFlagBits::eColor);
			mMotionVectorImages[i]->transition_image_layout(colorFormatMotionVectorImg, vk::ImageLayout::eUndefined, vk::ImageLayout::eColorAttachmentOptimal, 1);
			mMotionVectorTextures[i] = std::make_shared<cgb::vulkan_texture>(mMotionVectorImages[i]);
		}

	}

	void createVRSImageResources()
	{
		vk::Format colorFormat = vk::Format::eR8Uint;
		auto width = mImagePresenter->get_swap_chain_extent().width / cgb::vulkan_context::instance().shadingRateImageProperties.shadingRateTexelSize.width;
		auto height = mImagePresenter->get_swap_chain_extent().height / cgb::vulkan_context::instance().shadingRateImageProperties.shadingRateTexelSize.height;

		vk::Format colorFormatDebug = vk::Format::eR8G8B8A8Unorm;
		vk::Format colorFormatVrsPrevImg = vk::Format::eR16G16B16A16Unorm;
		vk::Format colorFormatMotionVectorImg = mMotionVectorImages[0]->get_format();


		vrsImages.resize(cgb::vulkan_context::instance().cgb::vulkan_context::instance().dynamicRessourceCount);
		vrsDefaultImage.resize(cgb::vulkan_context::instance().cgb::vulkan_context::instance().dynamicRessourceCount);

		vrsDebugImages.resize(cgb::vulkan_context::instance().cgb::vulkan_context::instance().dynamicRessourceCount);
		vrsDebugTextureImages.resize(cgb::vulkan_context::instance().cgb::vulkan_context::instance().dynamicRessourceCount);
		mVrsPrevRenderBlitImages.resize(cgb::vulkan_context::instance().cgb::vulkan_context::instance().dynamicRessourceCount);
		mVrsPrevRenderBlitTextures.resize(cgb::vulkan_context::instance().cgb::vulkan_context::instance().dynamicRessourceCount);
		mVrsMasMotionVecBlitImages.resize(cgb::vulkan_context::instance().cgb::vulkan_context::instance().dynamicRessourceCount);
		mVrsMasMotionVecBlitTextures.resize(cgb::vulkan_context::instance().cgb::vulkan_context::instance().dynamicRessourceCount);

		std::vector clearPixels = std::vector<uint8_t>(width * height, 7);

		for (int i = 0; i < cgb::vulkan_context::instance().cgb::vulkan_context::instance().dynamicRessourceCount; i++) {
			vrsImages[i] = std::make_shared<cgb::vulkan_image>(width, height, 1, 1, vk::SampleCountFlagBits::e1, colorFormat, vk::ImageTiling::eOptimal, 
				vk::ImageUsageFlagBits::eShadingRateImageNV | vk::ImageUsageFlagBits::eStorage, vk::MemoryPropertyFlagBits::eDeviceLocal, vk::ImageAspectFlagBits::eColor);
			vrsImages[i]->transition_image_layout(colorFormat, vk::ImageLayout::eUndefined, vk::ImageLayout::eShadingRateOptimalNV, 1); 

			vrsDefaultImage[i] = std::make_shared<cgb::vulkan_image>(width, height, 1, 1, vk::SampleCountFlagBits::e1, colorFormat, vk::ImageTiling::eOptimal,
				vk::ImageUsageFlagBits::eShadingRateImageNV | vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eTransferDst, vk::MemoryPropertyFlagBits::eDeviceLocal, vk::ImageAspectFlagBits::eColor);
			vrsDefaultImage[i]->update_pixels(clearPixels.data());
			vrsDefaultImage[i]->transition_image_layout(colorFormat, vk::ImageLayout::eUndefined, vk::ImageLayout::eShadingRateOptimalNV, 1);
			
			mVrsPrevRenderBlitImages[i] = std::make_shared<cgb::vulkan_image>(width, height, 1, 4, vk::SampleCountFlagBits::e1, colorFormatVrsPrevImg, vk::ImageTiling::eOptimal,
				vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eSampled, vk::MemoryPropertyFlagBits::eDeviceLocal, vk::ImageAspectFlagBits::eColor);
			mVrsPrevRenderBlitImages[i]->transition_image_layout(colorFormatVrsPrevImg, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal, 1);
			mVrsPrevRenderBlitTextures[i] = std::make_shared<cgb::vulkan_texture>(mVrsPrevRenderBlitImages[i]);

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

		width = mImagePresenter->get_swap_chain_extent().width;
		height = mImagePresenter->get_swap_chain_extent().height;
		mVrsPrevRenderImages.resize(cgb::vulkan_context::instance().cgb::vulkan_context::instance().dynamicRessourceCount);
		mVrsPrevRenderTextures.resize(cgb::vulkan_context::instance().cgb::vulkan_context::instance().dynamicRessourceCount);

		mVrsPrevRenderMsaaImage = std::make_shared<cgb::vulkan_image>(width, height, 1, 4, cgb::vulkan_context::instance().msaaSamples, colorFormatVrsPrevImg,
			vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eColorAttachment,
			vk::MemoryPropertyFlagBits::eDeviceLocal, vk::ImageAspectFlagBits::eColor);

		for (int i = 0; i < cgb::vulkan_context::instance().cgb::vulkan_context::instance().dynamicRessourceCount; i++) {
			mVrsPrevRenderImages[i] = std::make_shared<cgb::vulkan_image>(width, height, 1, 4, vk::SampleCountFlagBits::e1, colorFormatVrsPrevImg,
				vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferSrc,
				vk::MemoryPropertyFlagBits::eDeviceLocal, vk::ImageAspectFlagBits::eColor);
			mVrsPrevRenderImages[i]->transition_image_layout(colorFormatVrsPrevImg, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferSrcOptimal, 1);			
			mVrsPrevRenderTextures[i] = std::make_shared<cgb::vulkan_texture>(mVrsPrevRenderImages[i]);
		}
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

		auto posAndUv = std::make_shared<vulkan_attribute_description_binding>(1, sizeof(Vertex), vk::VertexInputRate::eVertex);
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
		mTAAIndices = std::vector<int>(mImagePresenter->get_swap_chain_images_count(), 0);

		for (int i = 0; i < mTAAFramebuffers.size(); i++) {
			mTAAFramebuffers[i] = std::make_shared<cgb::vulkan_framebuffer>(
				mImagePresenter->get_swap_chain_extent().width, mImagePresenter->get_swap_chain_extent().height,
				mImagePresenter->get_swap_chain_images_count(), vk::SampleCountFlagBits::e1);
			mTAAFramebuffers[i]->add_dynamic_color_attachment(mTAAImages[i], vk::ImageLayout::eShaderReadOnlyOptimal);
			mTAAFramebuffers[i]->bake();
		}

		// framebuffers are the same, therefore render passes are too and must be compatible! --> use render pass of first framebuffer
		mTAAPipeline = std::make_shared<cgb::vulkan_pipeline>(mTAAFramebuffers[0]->get_render_pass(), viewport, scissor, vk::SampleCountFlagBits::e1, std::vector<std::shared_ptr<cgb::vulkan_resource_bundle_layout>> { layout }, sizeof(vrs_cas_comp_data), cgb::ShaderStageFlagBits::eFragment);

		auto posAndUv = std::make_shared<vulkan_attribute_description_binding>(1, sizeof(Vertex), vk::VertexInputRate::eVertex);
		posAndUv->add_attribute_description(0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, pos));
		posAndUv->add_attribute_description(1, vk::Format::eR32G32Sfloat, offsetof(Vertex, texCoord));

		mTAAPipeline->add_attr_desc_binding(posAndUv);
		mTAAPipeline->add_shader(cgb::ShaderStageFlagBits::eVertex, "shaders/taa.vert.spv");
		mTAAPipeline->add_shader(cgb::ShaderStageFlagBits::eFragment, "shaders/taa.frag.spv");
		mTAAPipeline->disable_shading_rate_image();
		mTAAPipeline->bake();

		mTAADrawer = std::make_unique<cgb::vulkan_drawer>(drawCommandBufferManager, mTAAPipeline);
		mTAADrawer->set_vrs_images(vrsDefaultImage);
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

	void load_model(std::string inPath, glm::mat4 transform, const unsigned int model_loader_flags, std::unique_ptr<cgb::Model>& outModel)
	{
		outModel = cgb::Model::LoadFromFile(inPath, transform, mResourceBundleGroup, model_loader_flags);

		auto meshes = outModel->SelectAllMeshes();
		for (cgb::Mesh& mesh : meshes)
		{
			mesh.m_material_data->set_roughness(0.1f);
			mesh.m_material_data->set_albedo(glm::vec3(0.95f, 0.64f, 0.54f));
			mesh.m_material_data->set_metallic(1.0f);
			mesh.m_material_data->update_material_buffer();
		}

		for (cgb::Mesh& mesh : meshes) {
			auto outVertexBuffer = std::make_shared<cgb::vulkan_buffer>(sizeof(mesh.m_vertex_data[0]) * mesh.m_vertex_data.size(),
				vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer, mesh.m_vertex_data.data());
			auto outIndexBuffer = std::make_shared<cgb::vulkan_buffer>(sizeof(mesh.m_indices[0]) * mesh.m_indices.size(),
				vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer, mesh.m_indices.data());

			auto resourceBundle = mesh.mMaterialResourceBundle;

			auto sponzaRenderObject = std::make_shared<cgb::vulkan_render_object>(std::vector< std::shared_ptr<cgb::vulkan_buffer>>({ outVertexBuffer }), outIndexBuffer, mesh.m_indices.size(), mMaterialObjectResourceBundleLayout, mResourceBundleGroup, std::vector<std::shared_ptr<cgb::vulkan_resource_bundle>> {resourceBundle});

			mSponzaRenderObjects.push_back(sponzaRenderObject);
		}
		for (auto sponzaRenderObject : mSponzaRenderObjects) {
		//outModel->AllocateMaterialData();
			for (int i = 0; i < sponzaRenderObject->get_resource_bundles().size(); i++) {
				mResourceBundleGroup->allocate_resource_bundle(sponzaRenderObject->get_resource_bundles()[i].get());
			}
		}
	}

	/*! Position of the first/second point light, or the rotation origin when animating its position. */
	glm::vec3 kInitialPositionOfFirstPointLight = glm::vec3(-0.64f, 0.45f, 3.35f);
	glm::vec3 kInitialPositionOfSecondPointLight = glm::vec3(-0.05f, 2.12f, 0.53f);

	void create_lots_of_lights()
	{
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
		mainWnd->set_resolution({ 1600, 900 });
		mainWnd->set_presentaton_mode(cgb::presentation_mode::vsync);
		mainWnd->open();

		// Create a "behavior" which contains functionality of our program
		auto vrsBehavior = vrs_behavior();

		// Create a composition of all things that define the essence of 
		// our program, which there are:
		//  - a timer
		//  - an executor
		//  - a window
		//  - a behavior
		auto hello = cgb::composition<cgb::varying_update_timer, cgb::sequential_executor>({
					&vrsBehavior
				});

		// Let's go:
		hello.start();
	}
	catch (std::runtime_error & re) {
		LOG_ERROR_EM(re.what());
	}
}


