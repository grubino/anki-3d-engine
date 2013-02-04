#include "anki/renderer/MainRenderer.h"
#include "anki/core/Logger.h"
#include "anki/renderer/Deformer.h"
#include "anki/util/Filesystem.h"
#include <cstdlib>
#include <cstdio>
#include <fstream>

#define glewGetContext() (&glContext)

namespace anki {

//==============================================================================
MainRenderer::~MainRenderer()
{}

//==============================================================================
void MainRenderer::init(const Renderer::Initializer& initializer_)
{
	ANKI_LOGI("Initializing main renderer...");
	initGl();

	const F32 renderingQuality = initializer_.renderingQuality;
	windowWidth = initializer_.width;
	windowHeight = initializer_.height;
	drawToDefaultFbo = (renderingQuality > 0.9 && !initializer_.dbg.enabled);

	// init the offscreen Renderer
	//
	RendererInitializer initializer = initializer_;
	initializer.width *= renderingQuality;
	initializer.height *= renderingQuality;

	if(drawToDefaultFbo)
	{
		initializer.pps.drawToDefaultFbo = true;
		initializer.is.drawToDefaultFbo = !initializer.pps.enabled;
	}
	else
	{
		initializer.pps.drawToDefaultFbo = false;
		initializer.is.drawToDefaultFbo = false;
		sProg.load("shaders/Final.glsl");
	}

	Renderer::init(initializer);
	dbg.init(initializer);
	deformer.reset(new Deformer);

	ANKI_LOGI("Main renderer initialized");
}

//==============================================================================
void MainRenderer::initGl()
{
	/*glewExperimental = GL_TRUE;
	if(glewInit() != GLEW_OK)
	{
		throw ANKI_EXCEPTION("GLEW initialization failed");
	}*/

	// Ignore the first error
	glGetError();

	// print GL info
	ANKI_LOGI("OpenGL info: OGL "
		<< reinterpret_cast<const char*>(glGetString(GL_VERSION))
		<< ", GLSL " << reinterpret_cast<const char*>(
		glGetString(GL_SHADING_LANGUAGE_VERSION)));

	// get max texture units
	GlStateSingleton::get().setClearColor(Vec4(1.0, 0.0, 1.0, 1.0));
	GlStateSingleton::get().setClearDepthValue(1.0);
	GlStateSingleton::get().setClearStencilValue(0);
	glDepthFunc(GL_LEQUAL);
	// CullFace is always on
	glCullFace(GL_BACK);
	GlStateSingleton::get().enable(GL_CULL_FACE);

	// defaults
	GlStateSingleton::get().disable(GL_BLEND);
	GlStateSingleton::get().disable(GL_STENCIL_TEST);
	//glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	GlStateSingleton::get().setDepthMaskEnabled(true);
	glDepthFunc(GL_LESS);

	try
	{
		ANKI_CHECK_GL_ERROR();
	}
	catch(std::exception& e)
	{
		throw ANKI_EXCEPTION("OpenGL initialization failed") << e;
	}
}

//==============================================================================
void MainRenderer::render(Scene& scene)
{
	Renderer::render(scene);

	if(dbg.getEnabled())
	{
		dbg.run();
	}

	// Render the PPS FAI to the framebuffer
	//
	if(!drawToDefaultFbo)
	{
		Fbo::bindDefault(); // Bind the window framebuffer

		GlStateSingleton::get().setViewport(0, 0, windowWidth, windowHeight);
		GlStateSingleton::get().disable(GL_DEPTH_TEST);
		GlStateSingleton::get().disable(GL_BLEND);
		sProg->bind();
#if 0
		const Texture& finalFai = pps.getHdr().getFai();
#else
		const Texture& finalFai = pps.getFai();
#endif
		sProg->findUniformVariable("rasterImage").set(finalFai);
		drawQuad();
	}

	GLenum glerr = glGetError();
	if(glerr != GL_NO_ERROR)
	{
		throw ANKI_EXCEPTION("GL error");
	}
}

//==============================================================================
void MainRenderer::takeScreenshotTga(const char* filename)
{
	// open file and check
	std::fstream fs;
	fs.open(filename, std::ios::out | std::ios::binary);
	if(!fs.is_open())
	{
		throw ANKI_EXCEPTION("Cannot write screenshot file:"
			+ filename);
	}

	// write headers
	static const U8 tgaHeaderUncompressed[12] = {
		0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	Array<U8, 6> header;

	header[1] = getWidth() / 256;
	header[0] = getWidth() % 256;
	header[3] = getHeight() / 256;
	header[2] = getHeight() % 256;
	header[4] = 24;
	header[5] = 0;

	fs.write((char*)tgaHeaderUncompressed, sizeof(tgaHeaderUncompressed));
	fs.write((char*)&header[0], sizeof(header));

	// get the buffer
	Vector<U8> buffer;
	buffer.resize(getWidth() * getHeight() * 3);

	glReadPixels(0, 0, getWidth(), getHeight(), GL_RGB, GL_UNSIGNED_BYTE,
		&buffer[0]);

	for(U i = 0; i < getWidth() * getHeight() * 3; i += 3)
	{
		U8 temp = buffer[i];
		buffer[i] = buffer[i + 2];
		buffer[i + 2] = temp;
	}

	fs.write((char*)&buffer[0], getWidth() * getHeight() * 3);

	// end
	fs.close();
}

//==============================================================================
void MainRenderer::takeScreenshotJpeg(const char* filename)
{
#if 0
	// open file
	FILE* outfile = fopen(filename, "wb");

	if(!outfile)
	{
		throw ANKI_EXCEPTION("Cannot open file: " + filename);
	}

	// set jpg params
	jpeg_compress_struct cinfo;
	jpeg_error_mgr       jerr;

	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_compress(&cinfo);
	jpeg_stdio_dest(&cinfo, outfile);

	cinfo.image_width      = getWidth();
	cinfo.image_height     = getHeight();
	cinfo.input_components = 3;
	cinfo.in_color_space   = JCS_RGB;
	jpeg_set_defaults(&cinfo);
	jpeg_set_quality (&cinfo, screenshotJpegQuality, true);
	jpeg_start_compress(&cinfo, true);

	// read from OGL
	char* buffer = (char*)malloc(getWidth()*getHeight()*3*sizeof(char));
	glReadPixels(0, 0, getWidth(), getHeight(), GL_RGB, GL_UNSIGNED_BYTE,
		buffer);

	// write buffer to file
	JSAMPROW row_pointer;

	while(cinfo.next_scanline < cinfo.image_height)
	{
		row_pointer = (JSAMPROW)&buffer[(getHeight() - 1 -
			cinfo.next_scanline) * 3 * getWidth()];
		jpeg_write_scanlines(&cinfo, &row_pointer, 1);
	}

	jpeg_finish_compress(&cinfo);

	// done
	free(buffer);
	fclose(outfile);
#endif
}

//==============================================================================
void MainRenderer::takeScreenshot(const char* filename)
{
	std::string ext = getFileExtension(filename);

	// exec from this extension
	if(ext == "tga")
	{
		takeScreenshotTga(filename);
	}
	else if(ext == "jpg" || ext == "jpeg")
	{
		takeScreenshotJpeg(filename);
	}
	else
	{
		throw ANKI_EXCEPTION("Unsupported file extension: " + filename);
	}
	//ANKI_LOGI("Screenshot \"" << filename << "\" saved");
}

} // end namespace anki
