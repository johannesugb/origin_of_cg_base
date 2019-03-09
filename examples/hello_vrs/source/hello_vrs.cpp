// hello_vrs.cpp : Defines the entry point for the console application.
//
#include <tobii/tobii.h>
#include <tobii/tobii_streams.h>

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
#include "vrs_image_compute_drawer.h"
#include "eyetracking_interface.h"

#include "model.h"

const int WIDTH = 1920;
const int HEIGHT = 1080;

const std::string TEXTURE_PATH = "assets/chalet/chalet.jpg";

class vrs_behavior : public cgb::cg_element
{

public:
	vrs_behavior() {}
private:
	vk::DescriptorSetLayout descriptorSetLayout;
	vk::DescriptorPool descriptorPool;

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

	cgb::vulkan_render_object* renderObject;
	cgb::vulkan_render_object* renderObject2;
	cgb::vulkan_texture* texture;
	cgb::vulkan_image* textureImage;
	std::shared_ptr<cgb::vulkan_command_buffer_manager> drawCommandBufferManager;
	std::shared_ptr<cgb::vulkan_command_buffer_manager> transferCommandBufferManager;
	std::unique_ptr<cgb::vulkan_drawer> drawer;
	std::unique_ptr<cgb::vrs_image_compute_drawer> mVrsImageComputeDrawer;

	// render target needed for MSAA
	std::shared_ptr<cgb::vulkan_image> colorImage;
	std::shared_ptr<cgb::vulkan_image> depthImage;
	std::vector<std::shared_ptr<cgb::vulkan_image>> vrsImages;
	std::vector < std::shared_ptr<cgb::vulkan_image>> vrsDebugImages;
	std::vector < std::shared_ptr <cgb::vulkan_texture>> vrsDebugTextureImages;
	std::shared_ptr<cgb::vulkan_image_presenter> imagePresenter;
	std::shared_ptr<cgb::vulkan_render_queue> mVulkanRenderQueue;
	std::unique_ptr<cgb::vulkan_renderer> mRenderer;
	std::shared_ptr<cgb::vulkan_renderer> mVrsRenderer;
	std::shared_ptr<cgb::vulkan_pipeline> mRenderVulkanPipeline;
	std::shared_ptr<cgb::vulkan_pipeline> mComputeVulkanPipeline;
	std::shared_ptr<cgb::vulkan_framebuffer> mVulkanFramebuffer;

	std::shared_ptr<eyetracking_interface> eyeInf;


	std::unique_ptr<cgb::Model> mModel;

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
	}

	void render() override
	{
		static float sum_t = 0.0f;

		auto eyeData = eyeInf->get_eyetracking_data();

		drawFrame();

		sum_t += cgb::time().delta_time();
		if (sum_t >= 1.0f) {
			cgb::current_composition().window_in_focus()->set_title(std::to_string(1.0f / cgb::time().delta_time()).c_str());
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

		createCommandPools();

		transferCommandBufferManager = std::make_shared<cgb::vulkan_command_buffer_manager>(transferCommandPool, cgb::vulkan_context::instance().graphicsQueue);
		cgb::vulkan_context::instance().transferCommandBufferManager = transferCommandBufferManager;


		imagePresenter = std::make_shared<cgb::vulkan_image_presenter>(cgb::vulkan_context::instance().presentQueue, cgb::vulkan_context::instance().surface, cgb::vulkan_context::instance().findQueueFamilies());
		cgb::vulkan_context::instance().dynamicRessourceCount = imagePresenter->get_swap_chain_images_count();
		drawCommandBufferManager = std::make_shared<cgb::vulkan_command_buffer_manager>(imagePresenter->get_swap_chain_images_count(), commandPool, cgb::vulkan_context::instance().graphicsQueue);
		mVulkanRenderQueue = std::make_shared<cgb::vulkan_render_queue>(cgb::vulkan_context::instance().graphicsQueue);

		std::vector<std::shared_ptr<cgb::vulkan_renderer>> dependentRenderers = {};
		if (cgb::vulkan_context::instance().shadingRateImageSupported) {
			mVrsRenderer = std::make_shared<cgb::vulkan_renderer>(nullptr, mVulkanRenderQueue, drawCommandBufferManager, std::vector<std::shared_ptr<cgb::vulkan_renderer>>{}, true);
			dependentRenderers.push_back(mVrsRenderer);
		}
		mRenderer = std::make_unique<cgb::vulkan_renderer>(imagePresenter, mVulkanRenderQueue, drawCommandBufferManager, dependentRenderers);

		createColorResources();
		createDepthResources();
		if (cgb::vulkan_context::instance().shadingRateImageSupported) {
			createVRSImageResources();
		}

		mVulkanFramebuffer = std::make_shared<cgb::vulkan_framebuffer>(cgb::vulkan_context::instance().msaaSamples, colorImage, depthImage, imagePresenter);
		createDescriptorSetLayout();
		if (cgb::vulkan_context::instance().shadingRateImageSupported) {
			createVrsComputeDescriptorSetLayout();
		}

		vk::Viewport viewport = {};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = (float)imagePresenter->get_swap_chain_extent().width;
		viewport.height = (float)imagePresenter->get_swap_chain_extent().height;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		vk::Rect2D scissor = {};
		scissor.offset = { 0, 0 };
		scissor.extent = imagePresenter->get_swap_chain_extent();

		//Atribute description bindings
		auto bind1 = std::make_shared<vulkan_attribute_description_binding>(1, sizeof(Vertex), vk::VertexInputRate::eVertex);
		bind1->add_attribute_description(0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, pos));
		bind1->add_attribute_description(1, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, color));
		auto bind2 = std::make_shared<vulkan_attribute_description_binding>(2, sizeof(Vertex), vk::VertexInputRate::eVertex);
		bind2->add_attribute_description(2, vk::Format::eR32G32Sfloat, offsetof(Vertex, texCoord));

		// Render Drawer and Pipeline
		mRenderVulkanPipeline = std::make_shared<cgb::vulkan_pipeline>("shaders/triangle.vert.spv", "shaders/triangle.frag.spv", mVulkanFramebuffer->get_render_pass(), viewport, scissor, cgb::vulkan_context::instance().msaaSamples, descriptorSetLayout);
		mRenderVulkanPipeline->add_attr_desc_binding(bind1);
		mRenderVulkanPipeline->add_attr_desc_binding(bind2);
		mRenderVulkanPipeline->add_shader(cgb::ShaderStageFlagBits::eVertex, "shaders/triangle.vert.spv");
		mRenderVulkanPipeline->add_shader(cgb::ShaderStageFlagBits::eFragment, "shaders/triangle.frag.spv");

		mRenderVulkanPipeline->bake();
		drawer = std::make_unique<cgb::vulkan_drawer>(drawCommandBufferManager, mRenderVulkanPipeline);

		if (cgb::vulkan_context::instance().shadingRateImageSupported) {
			drawer->set_vrs_images(vrsImages);

			// Compute Drawer and Pipeline
			mComputeVulkanPipeline = std::make_shared<cgb::vulkan_pipeline>("shaders/vrs_img.comp.spv", std::vector<vk::DescriptorSetLayout> { vrsComputeDescriptorSetLayout }, sizeof(vrs_eye_comp_data));
			mComputeVulkanPipeline->add_shader(cgb::ShaderStageFlagBits::eCompute, "shaders/vrs_img.comp.spv");
			
			mVrsImageComputeDrawer = std::make_unique<cgb::vrs_image_compute_drawer>(drawCommandBufferManager, mComputeVulkanPipeline, vrsDebugImages);
			mVrsImageComputeDrawer->set_vrs_images(vrsImages);
		}


		createTexture();

		createDescriptorPool();

		if (cgb::vulkan_context::instance().shadingRateImageSupported) {
			createVrsComputeDescriptorPool();
			createVrsDescriptorSets();
			mVrsImageComputeDrawer->set_descriptor_sets(mVrsComputeDescriptorSets);
			mVrsImageComputeDrawer->set_width_height(vrsImages[0]->get_width(), vrsImages[0]->get_height());
			mVrsImageComputeDrawer->set_eye_inf(eyeInf);
		}

		renderObject = new cgb::vulkan_render_object(imagePresenter->get_swap_chain_images_count(), verticesQuad, indicesQuad, descriptorSetLayout, descriptorPool, texture, transferCommandBufferManager, vrsDebugTextureImages);
		renderObject2 = new cgb::vulkan_render_object(imagePresenter->get_swap_chain_images_count(), verticesScreenQuad, indicesScreenQuad, descriptorSetLayout, descriptorPool, texture, transferCommandBufferManager, vrsDebugTextureImages);


		// TODO transfer code to camera
		UniformBufferObject ubo = {};
		ubo.model = glm::mat4(1.0f);
		//ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		ubo.model[1][1] *= -1;
		ubo.mvp = ubo.model;
		renderObject2->update_uniform_buffer(0, ubo);
		renderObject2->update_uniform_buffer(1, ubo);
		renderObject2->update_uniform_buffer(2, ubo);

		load_model("assets/models/sponza/sponza_structure.obj", glm::scale(glm::vec3(0.01f)), cgb::MOLF_triangulate | cgb::MOLF_smoothNormals | cgb::MOLF_calcTangentSpace, mModel);
	}

	void cleanup()
	{
		cleanupSwapChain();

		cgb::vulkan_context::instance().device.destroyDescriptorPool(descriptorPool);
		cgb::vulkan_context::instance().device.destroyDescriptorSetLayout(descriptorSetLayout);
		cgb::vulkan_context::instance().device.destroyDescriptorPool(vrsComputeDescriptorPool);
		cgb::vulkan_context::instance().device.destroyDescriptorSetLayout(vrsComputeDescriptorSetLayout);

		delete renderObject;
		delete renderObject2;
		delete texture;
		delete textureImage;
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
		mRenderVulkanPipeline.reset();
		if (cgb::vulkan_context::instance().shadingRateImageSupported) {
			mVrsImageComputeDrawer.reset();
		}
		mComputeVulkanPipeline.reset();
		mVulkanFramebuffer.reset();

		imagePresenter.reset();
		if (cgb::vulkan_context::instance().shadingRateImageSupported) {
			mVrsRenderer.reset();
		}
		mRenderer.reset();
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
		mRenderer->start_frame();
		// update states, e.g. for animation
		static auto startTime = std::chrono::high_resolution_clock::now();

		auto currentTime = std::chrono::high_resolution_clock::now();
		float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
		UniformBufferObject ubo = {};
		ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		ubo.proj = glm::perspective(glm::radians(45.0f), imagePresenter->get_swap_chain_extent().width / (float)imagePresenter->get_swap_chain_extent().height, 0.1f, 10.0f);
		ubo.proj[1][1] *= -1;
		ubo.mvp = ubo.proj * ubo.view * ubo.model;
		renderObject->update_uniform_buffer(cgb::vulkan_context::instance().currentFrame, ubo);

		// start drawing, record draw commands, etc.
		cgb::vulkan_context::instance().vulkanFramebuffer = mVulkanFramebuffer;

		if (cgb::vulkan_context::instance().shadingRateImageSupported) {
			mVrsRenderer->render(std::vector<cgb::vulkan_render_object*>{}, mVrsImageComputeDrawer.get());
		}

		std::vector<cgb::vulkan_render_object*> renderObjects;
		//renderObjects.push_back(renderObject);
		renderObjects.push_back(renderObject2);
		//for (int i = 0; i < 1000; i++) {
		//	renderObjects.push_back(renderObject2);
		//}

		mRenderer->render(renderObjects, drawer.get());
		mRenderer->end_frame();
	}

	void createDescriptorSetLayout()
	{
		vk::DescriptorSetLayoutBinding uboLayoutBinding = {};
		uboLayoutBinding.binding = 0;
		uboLayoutBinding.descriptorType = vk::DescriptorType::eUniformBuffer;
		uboLayoutBinding.descriptorCount = 1;
		uboLayoutBinding.stageFlags = vk::ShaderStageFlagBits::eVertex;
		uboLayoutBinding.pImmutableSamplers = nullptr; // Optional

		vk::DescriptorSetLayoutBinding samplerLayoutBinding = {};
		samplerLayoutBinding.binding = 1;
		samplerLayoutBinding.descriptorCount = 1;
		samplerLayoutBinding.descriptorType = vk::DescriptorType::eCombinedImageSampler;
		samplerLayoutBinding.pImmutableSamplers = nullptr;
		samplerLayoutBinding.stageFlags = vk::ShaderStageFlagBits::eFragment;

		std::vector<vk::DescriptorSetLayoutBinding> bindings = { uboLayoutBinding, samplerLayoutBinding };
		if (cgb::vulkan_context::instance().shadingRateImageSupported) {
			vk::DescriptorSetLayoutBinding samplerDebugLayoutBinding = {};
			samplerDebugLayoutBinding.binding = 2;
			samplerDebugLayoutBinding.descriptorCount = 1;
			samplerDebugLayoutBinding.descriptorType = vk::DescriptorType::eCombinedImageSampler;
			samplerDebugLayoutBinding.pImmutableSamplers = nullptr;
			samplerDebugLayoutBinding.stageFlags = vk::ShaderStageFlagBits::eFragment;

			bindings.push_back(samplerDebugLayoutBinding);
		}

		vk::DescriptorSetLayoutCreateInfo layoutInfo = {};
		layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
		layoutInfo.pBindings = bindings.data();


		if (cgb::vulkan_context::instance().device.createDescriptorSetLayout(&layoutInfo, nullptr, &descriptorSetLayout) != vk::Result::eSuccess) {
			throw std::runtime_error("failed to create descriptor set layout!");
		}
	}

	void createDescriptorPool()
	{
		std::vector<vk::DescriptorPoolSize> poolSizes(2);
		poolSizes[0].type = vk::DescriptorType::eUniformBuffer;
		poolSizes[0].descriptorCount = static_cast<uint32_t>(imagePresenter->get_swap_chain_images_count()) * 2;
		poolSizes[1].type = vk::DescriptorType::eCombinedImageSampler;
		poolSizes[1].descriptorCount = static_cast<uint32_t>(imagePresenter->get_swap_chain_images_count()) * 2;
		if (cgb::vulkan_context::instance().shadingRateImageSupported) {
			poolSizes.resize(3);
			poolSizes[2].type = vk::DescriptorType::eCombinedImageSampler;
			poolSizes[2].descriptorCount = static_cast<uint32_t>(imagePresenter->get_swap_chain_images_count()) * 2;
		}

		vk::DescriptorPoolCreateInfo poolInfo = {};
		poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
		poolInfo.pPoolSizes = poolSizes.data();
		poolInfo.maxSets = static_cast<uint32_t>(imagePresenter->get_swap_chain_images_count() * 2);

		if (cgb::vulkan_context::instance().device.createDescriptorPool(&poolInfo, nullptr, &descriptorPool) != vk::Result::eSuccess) {
			throw std::runtime_error("failed to create descriptor pool!");
		}
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
		poolSizes[0].descriptorCount = static_cast<uint32_t>(imagePresenter->get_swap_chain_images_count());

		vk::DescriptorPoolCreateInfo poolInfo = {};
		poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
		poolInfo.pPoolSizes = poolSizes.data();
		poolInfo.maxSets = static_cast<uint32_t>(imagePresenter->get_swap_chain_images_count());

		if (cgb::vulkan_context::instance().device.createDescriptorPool(&poolInfo, nullptr, &vrsComputeDescriptorPool) != vk::Result::eSuccess) {
			throw std::runtime_error("failed to create vrs compute descriptor pool!");
		}
	}

	void createVrsDescriptorSets()
	{
		std::vector<vk::DescriptorSetLayout> layouts(imagePresenter->get_swap_chain_images_count(), vrsComputeDescriptorSetLayout);
		vk::DescriptorSetAllocateInfo allocInfo = {};
		allocInfo.descriptorPool = vrsComputeDescriptorPool;
		allocInfo.descriptorSetCount = static_cast<uint32_t>(imagePresenter->get_swap_chain_images_count());
		allocInfo.pSetLayouts = layouts.data();

		mVrsComputeDescriptorSets.resize(imagePresenter->get_swap_chain_images_count());
		if (cgb::vulkan_context::instance().device.allocateDescriptorSets(&allocInfo, mVrsComputeDescriptorSets.data()) != vk::Result::eSuccess) {
			throw std::runtime_error("failed to allocate descriptor sets!");
		}

		for (size_t i = 0; i < imagePresenter->get_swap_chain_images_count(); i++) {
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
		textureImage = new cgb::vulkan_image(transferCommandBufferManager, pixels, texWidth, texHeight, texChannels);
		texture = new cgb::vulkan_texture(textureImage);

		stbi_image_free(pixels);
	}

	// attachments for framebuffer (color image to render to before resolve, depth image)

	void createDepthResources()
	{
		vk::Format depthFormat = findDepthFormat();

		depthImage = std::make_shared<cgb::vulkan_image>(transferCommandBufferManager, imagePresenter->get_swap_chain_extent().width, imagePresenter->get_swap_chain_extent().height, 1, cgb::vulkan_context::instance().msaaSamples, depthFormat,
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
		vk::Format colorFormat = imagePresenter->get_swap_chain_image_format();

		colorImage = std::make_shared<cgb::vulkan_image>(transferCommandBufferManager, imagePresenter->get_swap_chain_extent().width, imagePresenter->get_swap_chain_extent().height, 1, cgb::vulkan_context::instance().msaaSamples, colorFormat,
												  vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eTransientAttachment | vk::ImageUsageFlagBits::eColorAttachment, vk::MemoryPropertyFlagBits::eDeviceLocal, vk::ImageAspectFlagBits::eColor);
		colorImage->transition_image_layout(colorFormat, vk::ImageLayout::eUndefined, vk::ImageLayout::eColorAttachmentOptimal, 1);
	}

	void createVRSImageResources()
	{
		vk::Format colorFormat = vk::Format::eR8Uint;
		auto width = imagePresenter->get_swap_chain_extent().width / cgb::vulkan_context::instance().shadingRateImageProperties.shadingRateTexelSize.width;
		auto height = imagePresenter->get_swap_chain_extent().height / cgb::vulkan_context::instance().shadingRateImageProperties.shadingRateTexelSize.height;

		vk::Format colorFormatDebug = imagePresenter->get_swap_chain_image_format();

		vrsImages.resize(cgb::vulkan_context::instance().cgb::vulkan_context::instance().dynamicRessourceCount);
		vrsDebugImages.resize(cgb::vulkan_context::instance().cgb::vulkan_context::instance().dynamicRessourceCount);
		vrsDebugTextureImages.resize(cgb::vulkan_context::instance().cgb::vulkan_context::instance().dynamicRessourceCount);
		for (int i = 0; i < cgb::vulkan_context::instance().cgb::vulkan_context::instance().dynamicRessourceCount; i++) {
			vrsImages[i] = std::make_shared<cgb::vulkan_image>(transferCommandBufferManager, width, height, 1, vk::SampleCountFlagBits::e1, colorFormat,
														vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eShadingRateImageNV | vk::ImageUsageFlagBits::eStorage, vk::MemoryPropertyFlagBits::eDeviceLocal, vk::ImageAspectFlagBits::eColor);
			vrsImages[i]->transition_image_layout(colorFormat, vk::ImageLayout::eUndefined, vk::ImageLayout::eShadingRateOptimalNV, 1); // vk::ImageLayout::eShadingRateOptimalNV

			// Debug image
			vrsDebugImages[i] = std::make_shared<cgb::vulkan_image>(transferCommandBufferManager, width, height, 1, vk::SampleCountFlagBits::e1, colorFormatDebug,
															 vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eSampled, vk::MemoryPropertyFlagBits::eDeviceLocal, vk::ImageAspectFlagBits::eColor);
			vrsDebugImages[i]->transition_image_layout(colorFormatDebug, vk::ImageLayout::eUndefined, vk::ImageLayout::eShaderReadOnlyOptimal, 1);

			vrsDebugTextureImages[i] = std::make_shared <cgb::vulkan_texture>(vrsDebugImages[i].get());
		}
	}

	void load_model(std::string inPath, glm::mat4 transform, const unsigned int model_loader_flags, std::unique_ptr<cgb::Model>& outModel)
	{
		outModel = cgb::Model::LoadFromFile(inPath, transform, model_loader_flags);
		auto& mesh = outModel->mesh_at(0);

		auto outVertexBuffer = std::make_shared<cgb::vulkan_buffer>(sizeof(mesh.m_vertex_data[0]) * mesh.m_vertex_data.size(),
			vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer, mesh.m_vertex_data.data());
		auto outIndexBuffer = std::make_shared<cgb::vulkan_buffer>(sizeof(mesh.m_indices[0]) * mesh.m_indices.size(),
			vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer, mesh.m_indices.data());

		auto testObj = cgb::vulkan_render_object({outVertexBuffer}, outIndexBuffer, mesh.m_indices.size());
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
		auto windowParams = cgb::window_params{
			std::nullopt,
			std::nullopt,
			"Hello VRS World!"
		};
		auto mainWnd = cgb::context().create_window(windowParams, cgb::swap_chain_params{});

		// Create a "behavior" which contains functionality of our program
		auto vrsBehavior = vrs_behavior();

		// Create a composition of all things that define the essence of 
		// our program, which there are:
		//  - a timer
		//  - an executor
		//  - a window
		//  - a behavior
		auto hello = cgb::composition<cgb::varying_update_only_timer, cgb::sequential_executor>({
				mainWnd
				}, {
					&vrsBehavior
				});

		// Let's go:
		hello.start();
	}
	catch (std::runtime_error & re) {
		LOG_ERROR_EM(re.what());
	}
}


