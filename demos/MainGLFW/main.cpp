/*
 Copyright (c) 2012 The KoRE Project

  This file is part of KoRE.

  KoRE is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  KoRE is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with KoRE.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <GL/glew.h>
#include <GL/glfw.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <string>
#include <ctime>
#include <vector>

#include "KoRE/GLerror.h"
#include "KoRE/ShaderProgram.h"
#include "KoRE/Components/MeshComponent.h"
#include "KoRE/Components/TexturesComponent.h"
#include "KoRE/Operations/RenderMesh.h"
#include "KoRE/Operations/BindOperations/BindAttribute.h"
#include "KoRE/Operations/BindOperations/BindUniform.h"
#include "KoRE/Operations/BindOperations/BindTexture.h"
#include "KoRE/Operations/UseFBO.h"
#include "KoRE/Operations/UseShaderProgram.h"
#include "KoRE/ResourceManager.h"
#include "KoRE/RenderManager.h"
#include "KoRE/Components/Camera.h"
#include "KoRE/SceneNode.h"
#include "KoRE/Timer.h"
#include "KoRE/Texture.h"
#include "KoRE/FrameBuffer.h"
#include "Kore/Passes/FrameBufferStage.h"
#include "Kore/Passes/ShaderProgramPass.h"
#include "KoRE/Passes/NodePass.h"
#include "KoRE/Events.h"
#include "Kore/Operations/OperationFactory.h"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
#include <libavformat/avformat.h>
#include <libavutil/mem.h>
#include <libavutil/opt.h>
#include <libavutil/channel_layout.h>
#include <libavutil/common.h>
#include <libavutil/imgutils.h>
#include <libavutil/mathematics.h>
#include <libavutil/samplefmt.h>
}

kore::SceneNode* rotationNode = NULL;
kore::SceneNode* lightNode = NULL;
kore::Camera* pCamera = NULL;

uint8_t* colorbuffer;
AVOutputFormat* fmt;
AVFormatContext* oc;
AVCodec* codec;
AVStream* st;
AVCodecContext* c;
int video_outbuf_size;
uint8_t *video_outbuf;
AVFrame *picture, *tmp_picture;
struct SwsContext* converter;
/*void CALLBACK DebugLog(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, GLvoid* userParams) {
  //Log::getInstance()->write("[ERROR] Type: %s, Source: %s, Severity: %s\n", 
    //  glGetStringForType(type).c_str(),)
  kore::Log::getInstance()->write("[GL-ERROR]\n");
} */

//awaits simple shader

/* openglFFMPEG

static AVFrame *alloc_picture(AVPixelFormat pix_fmt, int width, int height)
{
  AVFrame *picture;
  uint8_t *picture_buf;

  picture = avcodec_alloc_frame();
  if (!picture) return NULL;

  picture->format = c->pix_fmt;
  picture->width  = c->width;
  picture->height = c->height;

  const size_t size = avpicture_get_size(pix_fmt, width, height);
  picture_buf = static_cast<uint8_t *>(av_malloc(size));

  if (!picture_buf) {
    av_free(picture);
    return NULL;
  }
  avpicture_fill((AVPicture *)picture, picture_buf, pix_fmt, width, height);
  return picture;
}

void initffmpeg(const char* filename,unsigned int width,unsigned int height, unsigned int framerate){
 
  int buffersize = 800*600*4;
  colorbuffer = new uint8_t[buffersize];
  memset(colorbuffer,255,buffersize);
  
  av_register_all();
  avcodec_register_all();

  avformat_alloc_output_context2(&oc, NULL, NULL, filename);
  if (!oc) {
    printf("Could not deduce output format from file extension: using MPEG.\n");
    avformat_alloc_output_context2(&oc, NULL, "mp4", filename);
  }
  if (!oc) {
    exit(1);
  }
  fmt = oc->oformat;
 
  codec = avcodec_find_encoder(CODEC_ID_H264);
  if (!codec) {
    fprintf(stderr, "codec not found\n");
    exit(1);
  }
  st = avformat_new_stream(oc, codec);
  if (!st) {
    fprintf(stderr, "Could not alloc stream\n");
    exit(1);
  }
  st->id = 1;

  c = st->codec;
  c->codec_id = CODEC_ID_H264;
  c->codec_type = AVMEDIA_TYPE_VIDEO;
  //c->bit_rate = 400000;
  c->width = width;
  c->height = height;
  c->time_base.den = framerate;
  c->time_base.num = 1;
  //c->gop_size = 12; / * emit one intra frame every twelve frames at most * /
  //c->gop_size = 25;
  c->pix_fmt = PIX_FMT_YUV420P;
  if(oc->oformat->flags & AVFMT_GLOBALHEADER)
    c->flags |= CODEC_FLAG_GLOBAL_HEADER;
     
      //?av_set_parameters?
  if (avformat_write_header(oc, NULL) < 0) {
    fprintf(stderr, "Invalid output format parameters\n");
    exit(1);
  }
  av_dump_format(oc,0,filename,1);
  
  AVDictionary* conf = NULL;
  av_dict_set(&conf, "crf", "0", 0);
  av_dict_set(&conf, "preset", "veryslow", 0);
  if (avcodec_open2(c, codec, &conf) < 0) {
    fprintf(stderr, "could not open codec\n");
    exit(1);
  }

  video_outbuf_size = 2000000;
  video_outbuf = (uint8_t*)(video_outbuf_size);

  picture = alloc_picture(c->pix_fmt, c->width, c->height);
  if (!picture) {
    fprintf(stderr, "Could not allocate picture\n");
    exit(1);
  }

  tmp_picture = alloc_picture(PIX_FMT_BGRA, width, height);
  if (!tmp_picture) {
    fprintf(stderr, "Could not allocate temporary picture\n");
    exit(1);
  }

  if (avio_open(&oc->pb, filename, AVIO_FLAG_WRITE) < 0) {
    fprintf(stderr, "Could not open '%s'\n", filename);
    exit(1);
  }

  / * write the stream header, if any * /
  avformat_write_header(oc, NULL);

  converter = sws_getContext(
    width, height, PIX_FMT_BGRA,
    width, height, c->pix_fmt,
    SWS_FAST_BILINEAR, NULL, NULL, NULL
    );
}

int readbuffer(){
  glReadBuffer(GL_BACK); 
  glReadPixels(0,0,800,600,GL_BGRA,GL_UNSIGNED_BYTE,colorbuffer);
    
  const uint8_t* data = colorbuffer + 800*600*4;
  const uint8_t* tmp[4] = { data, NULL, NULL, NULL };
  int stride[4] = { -800*4, 0, 0, 0 };

  sws_scale(converter, tmp, stride,
    0, 600, picture->data, picture->linesize);

  static int64_t frame_counter = 0;
  picture->pts = frame_counter++;

  const size_t out_size = avcodec_encode_video(c, video_outbuf, video_outbuf_size, picture);

  / * If out_size is zero the data was buffered * /
  if ( out_size == 0 ){
    return 0;
  }

  AVPacket pkt;
  av_init_packet(&pkt);
  pkt.pts = pkt.dts = AV_NOPTS_VALUE;

  if (c->coded_frame->pts != AV_NOPTS_VALUE){
    pkt.pts = av_rescale_q(c->coded_frame->pts, c->time_base, st->time_base);
  }

  if(c->coded_frame->key_frame)
    pkt.flags |= AV_PKT_FLAG_KEY;

  pkt.stream_index = st->index;
  pkt.data = video_outbuf;
  pkt.size = out_size;

  return av_write_frame(oc, &pkt);
}*/

/* Add an output stream. */
static AVStream *add_stream(AVFormatContext *oc, AVCodec **codec,
                            enum AVCodecID codec_id)
{
    AVCodecContext *c;
    AVStream *st;

    /* find the encoder */
    *codec = avcodec_find_encoder(codec_id);
    if (!(*codec)) {
        fprintf(stderr, "Could not find encoder for '%s'\n",
                avcodec_get_name(codec_id));
        exit(1);
    }

    st = avformat_new_stream(oc, *codec);
    if (!st) {
        fprintf(stderr, "Could not allocate stream\n");
        exit(1);
    }
    st->id = oc->nb_streams-1;
    c = st->codec;

    switch ((*codec)->type) {
    case AVMEDIA_TYPE_AUDIO:
        st->id = 1;
        c->sample_fmt  = AV_SAMPLE_FMT_S16;
        c->bit_rate    = 64000;
        c->sample_rate = 44100;
        c->channels    = 2;
        break;

    case AVMEDIA_TYPE_VIDEO:
        c->codec_id = codec_id;

        c->bit_rate = 400000;
        /* Resolution must be a multiple of two. */
        c->width    = 352;
        c->height   = 288;
        /* timebase: This is the fundamental unit of time (in seconds) in terms
         * of which frame timestamps are represented. For fixed-fps content,
         * timebase should be 1/framerate and timestamp increments should be
         * identical to 1. */
        c->time_base.den = STREAM_FRAME_RATE;
        c->time_base.num = 1;
        c->gop_size      = 12; /* emit one intra frame every twelve frames at most */
        c->pix_fmt       = STREAM_PIX_FMT;
        if (c->codec_id == AV_CODEC_ID_MPEG2VIDEO) {
            /* just for testing, we also add B frames */
            c->max_b_frames = 2;
        }
        if (c->codec_id == AV_CODEC_ID_MPEG1VIDEO) {
            /* Needed to avoid using macroblocks in which some coeffs overflow.
             * This does not happen with normal video, it just happens here as
             * the motion of the chroma plane does not match the luma plane. */
            c->mb_decision = 2;
        }
    break;

    default:
        break;
    }

    /* Some formats want stream headers to be separate. */
    if (oc->oformat->flags & AVFMT_GLOBALHEADER)
        c->flags |= CODEC_FLAG_GLOBAL_HEADER;

    return st;
}
static AVFrame *frame;
static AVPicture src_picture, dst_picture;
static int frame_count;

static void open_video(AVFormatContext *oc, AVCodec *codec, AVStream *st)
{
    int ret;
    AVCodecContext *c = st->codec;

    /* open the codec */
    ret = avcodec_open2(c, codec, NULL);
    if (ret < 0) {
        fprintf(stderr, "Could not open video codec: %s\n");
        exit(1);
    }

    /* allocate and init a re-usable frame */
    frame = avcodec_alloc_frame();
    if (!frame) {
        fprintf(stderr, "Could not allocate video frame\n");
        exit(1);
    }

    /* Allocate the encoded raw picture. */
    ret = avpicture_alloc(&dst_picture, c->pix_fmt, c->width, c->height);
    if (ret < 0) {
        fprintf(stderr, "Could not allocate picture: %s\n");
        exit(1);
    }

    /* If the output format is not YUV420P, then a temporary YUV420P
     * picture is needed too. It is then converted to the required
     * output format. */
    if (c->pix_fmt != AV_PIX_FMT_YUV420P) {
        ret = avpicture_alloc(&src_picture, AV_PIX_FMT_YUV420P, c->width, c->height);
        if (ret < 0) {
            fprintf(stderr, "Could not allocate temporary picture: %s\n");
            exit(1);
        }
    }

    /* copy data and linesize picture pointers to frame */
    *((AVPicture *)frame) = dst_picture;
}

static void write_video_frame(AVFormatContext *oc, AVStream *st)
{
    int ret;
    static struct SwsContext *sws_ctx;
    AVCodecContext *c = st->codec;

    if (frame_count >= STREAM_NB_FRAMES) {
        /* No more frames to compress. The codec has a latency of a few
         * frames if using B-frames, so we get the last frames by
         * passing the same picture again. */
    } else {
        if (c->pix_fmt != AV_PIX_FMT_YUV420P) {
            /* as we only generate a YUV420P picture, we must convert it
             * to the codec pixel format if needed */
            if (!sws_ctx) {
                sws_ctx = sws_getContext(c->width, c->height, AV_PIX_FMT_YUV420P,
                                         c->width, c->height, c->pix_fmt,
                                         sws_flags, NULL, NULL, NULL);
                if (!sws_ctx) {
                    fprintf(stderr,
                            "Could not initialize the conversion context\n");
                    exit(1);
                }
            }
            fill_yuv_image(&src_picture, frame_count, c->width, c->height);
            sws_scale(sws_ctx,
                      (const uint8_t * const *)src_picture.data, src_picture.linesize,
                      0, c->height, dst_picture.data, dst_picture.linesize);
        } else {
            fill_yuv_image(&dst_picture, frame_count, c->width, c->height);
        }
    }

    if (oc->oformat->flags & AVFMT_RAWPICTURE) {
        /* Raw video case - directly store the picture in the packet */
        AVPacket pkt;
        av_init_packet(&pkt);

        pkt.flags        |= AV_PKT_FLAG_KEY;
        pkt.stream_index  = st->index;
        pkt.data          = dst_picture.data[0];
        pkt.size          = sizeof(AVPicture);

        ret = av_interleaved_write_frame(oc, &pkt);
    } else {
        AVPacket pkt = { 0 };
        int got_packet;
        av_init_packet(&pkt);

        /* encode the image */
        ret = avcodec_encode_video2(c, &pkt, frame, &got_packet);
        if (ret < 0) {
            fprintf(stderr, "Error encoding video frame: %s\n");
            exit(1);
        }
        /* If size is zero, it means the image was buffered. */

        if (!ret && got_packet && pkt.size) {
            pkt.stream_index = st->index;

            /* Write the compressed frame to the media file. */
            ret = av_interleaved_write_frame(oc, &pkt);
        } else {
            ret = 0;
        }
    }
    if (ret != 0) {
        fprintf(stderr, "Error while writing video frame: %s\n");
        exit(1);
    }
    frame_count++;
}

static void close_video(AVFormatContext *oc, AVStream *st)
{
  avcodec_close(st->codec);
  av_free(src_picture.data[0]);
  av_free(dst_picture.data[0]);
  av_free(frame);
}

static int initEncode(const char* filename){
  AVOutputFormat *fmt;
  AVFormatContext *oc;
  AVStream *video_st;
  AVCodec *video_codec;
  double video_pts;
  int ret;

  av_register_all();

  avformat_alloc_output_context2(&oc, NULL, NULL, filename);
  if (!oc) {
    printf("Could not deduce output format from file extension: using MPEG.\n");
    avformat_alloc_output_context2(&oc, NULL, "mpeg", filename);
  }
  if (!oc) {
    return 1;
  }
  fmt = oc->oformat;

   /* Add the audio and video streams using the default format codecs
    * and initialize the codecs. */
    video_st = NULL;
  if (fmt->video_codec != AV_CODEC_ID_NONE) {
    video_st = add_stream(oc, &video_codec, fmt->video_codec);
  }
}


void setUpSimpleRendering(kore::SceneNode* renderNode, kore::ShaderProgramPass*
                          programPass, kore::Texture* texture, 
                          kore::LightComponent* light){

        kore::NodePass* nodePass = new kore::NodePass;
        const kore::ShaderProgram* simpleShader = 
            programPass->getShaderProgram();
        kore::MeshComponent* pMeshComponent =
            static_cast<kore::MeshComponent*>
            (renderNode->getComponent(kore::COMPONENT_MESH));

        // Add Texture
        kore::GLerror::gl_ErrorCheckStart();
        kore::TexturesComponent* pTexComponent = new kore::TexturesComponent;
        pTexComponent->addTexture(texture);
        renderNode->addComponent(pTexComponent);
        kore::GLerror::gl_ErrorCheckFinish("Initialization");

              
        kore::BindAttribute* normAttBind =
            new kore::BindAttribute(pMeshComponent->getShaderData("v_normal"),
            simpleShader->getAttribute("v_normal"));

        kore::BindAttribute* uvAttBind =
            new kore::BindAttribute(pMeshComponent->getShaderData("v_uv0"),
            simpleShader->getAttribute("v_uv0"));

        // Bind Uniform-Ops
        kore::BindUniform* modelBind = 
            new kore::BindUniform(renderNode->getTransform()->
            getShaderData("model Matrix"), simpleShader->getUniform("model"));

        kore::BindUniform* viewBind =
            new kore::BindUniform(pCamera->getShaderData("view Matrix"),
            simpleShader->getUniform("view"));

        kore::BindUniform* projBind =
            new kore::BindUniform(pCamera->getShaderData("projection Matrix"),
            simpleShader->getUniform("projection"));

        kore::BindTexture* texBind =
            new kore::BindTexture(pTexComponent->
            getShaderData(texture->getName()),
            simpleShader->getUniform("tex"));

        kore::BindUniform* lightPosBind =
            new kore::BindUniform(light->getShaderData("position"),
            simpleShader->getUniform("pointlightPos"));

        kore::RenderMesh* pRenderOp = new kore::RenderMesh();
        pRenderOp->connect(pMeshComponent, simpleShader);

        nodePass->addOperation(kore::OperationFactory::create(kore::OP_BINDATTRIBUTE, "v_position", pMeshComponent, "v_position", simpleShader));
        nodePass->addOperation(normAttBind);
        nodePass->addOperation(uvAttBind);
        nodePass->addOperation(modelBind);
        nodePass->addOperation(viewBind);
        nodePass->addOperation(projBind);
        nodePass->addOperation(lightPosBind);
        nodePass->addOperation(texBind);
        nodePass->addOperation(pRenderOp);

        programPass->addNodePass(nodePass);
}

void setUpNMRendering(kore::SceneNode* renderNode, 
                      kore::ShaderProgramPass* programPass, 
                      kore::Texture* texture,
                      kore::Texture* normalmap,
                      kore::LightComponent* light) {

        kore::NodePass* nodePass = new kore::NodePass;
        const kore::ShaderProgram* nmShader = 
            programPass->getShaderProgram();
        kore::MeshComponent* pMeshComponent =
            static_cast<kore::MeshComponent*>
            (renderNode->getComponent(kore::COMPONENT_MESH));

        // Add Texture
        kore::GLerror::gl_ErrorCheckStart();
        kore::TexturesComponent* pTexComponent = new kore::TexturesComponent;
        pTexComponent->addTexture(texture);
        pTexComponent->addTexture(normalmap);
        renderNode->addComponent(pTexComponent);
        kore::GLerror::gl_ErrorCheckFinish("Initialization");

        // Bind Attribute-Ops
        kore::BindAttribute* posAttBind =
            new kore::BindAttribute(pMeshComponent->getShaderData("v_position"),
            nmShader->getAttribute("v_position"));

        kore::BindAttribute* normAttBind =
            new kore::BindAttribute(pMeshComponent->getShaderData("v_normal"),
            nmShader->getAttribute("v_normal"));

        kore::BindAttribute* tangentAttBind =
            new kore::BindAttribute(pMeshComponent->getShaderData("v_tangent"),
            nmShader->getAttribute("v_tangent"));

        kore::BindAttribute* uvAttBind =
            new kore::BindAttribute(pMeshComponent->getShaderData("v_uv0"),
            nmShader->getAttribute("v_uv0"));

        // Bind Uniform-Ops
        kore::BindUniform* modelBind = 
            new kore::BindUniform(renderNode->getTransform()->
            getShaderData("model Matrix"), nmShader->getUniform("model"));

        kore::BindUniform* normalMatBind = 
            new kore::BindUniform(renderNode->getTransform()->
            getShaderData("normal Matrix"), nmShader->getUniform("normal"));

        kore::BindUniform* viewBind =
            new kore::BindUniform(pCamera->getShaderData("view Matrix"),
            nmShader->getUniform("view"));

        kore::BindUniform* invViewBind =
          new kore::BindUniform(pCamera->getShaderData("inverse view Matrix"),
          nmShader->getUniform("viewI"));

        kore::BindUniform* projBind =
            new kore::BindUniform(pCamera->getShaderData("projection Matrix"),
            nmShader->getUniform("projection"));


        kore::BindTexture* texBind =
            new kore::BindTexture(pTexComponent->
            getShaderData(texture->getName()),
            nmShader->getUniform("tex"));

        kore::BindTexture* normalmapBind =
            new kore::BindTexture(pTexComponent->
            getShaderData(normalmap->getName()),
            nmShader->getUniform("normalmap"));

        kore::BindUniform* lightPosBind =
            new kore::BindUniform(light->getShaderData("position"),
            nmShader->getUniform("pointlightPos"));

        kore::RenderMesh* pRenderOp = new kore::RenderMesh();
        pRenderOp->connect(pMeshComponent, nmShader);

        nodePass->addOperation(posAttBind);
        nodePass->addOperation(normAttBind);
        nodePass->addOperation(tangentAttBind);
        nodePass->addOperation(uvAttBind);
        nodePass->addOperation(modelBind);
        nodePass->addOperation(normalMatBind);
        nodePass->addOperation(viewBind);
        nodePass->addOperation(invViewBind);
        nodePass->addOperation(projBind);
        nodePass->addOperation(lightPosBind);
        nodePass->addOperation(texBind);
        nodePass->addOperation(normalmapBind);
        nodePass->addOperation(pRenderOp);

        programPass->addNodePass(nodePass);
}

int main(void) {

  int running = GL_TRUE;

  // Initialize GLFW
  if (!glfwInit()) {
    kore::Log::getInstance()->write("[ERROR] could not load window manager\n");
    exit(EXIT_FAILURE);
  }

  /*glfwOpenWindowHint(GLFW_OPENGL_VERSION_MAJOR, 4);
  glfwOpenWindowHint(GLFW_OPENGL_VERSION_MINOR, 2);
  glfwOpenWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwOpenWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);*/

  // Open an OpenGL window
  if (!glfwOpenWindow(800, 600, 8, 8, 8, 8, 24, 8, GLFW_WINDOW)) {
    kore::Log::getInstance()->write("[ERROR] could not open render window\n");
    glfwTerminate();
    exit(EXIT_FAILURE);
  }

  // disable v-sync
  glfwSwapInterval(0);

  // initialize GLEW
  glewExperimental = GL_TRUE;
  if (glewInit()) {
    kore::Log::getInstance()->write("[ERROR] could not open initialize extension manager\n");
    glfwTerminate();
    exit(EXIT_FAILURE);
  }

  // Init gl-states
  // glEnable(GL_VERTEX_ARRAY);
    
  // log versions
  int GLFWmajor, GLFWminor, GLFWrev;
  glfwGetVersion(&GLFWmajor, &GLFWminor, &GLFWrev);
  kore::Log::getInstance()
    ->write("Render Device: %s\n",
            reinterpret_cast<const char*>(
            glGetString(GL_RENDERER)));
  kore::Log::getInstance()
    ->write("Vendor: %s\n",
            reinterpret_cast<const char*>(
            glGetString(GL_VENDOR)));
  kore::Log::getInstance()
    ->write("OpenGL version: %s\n",
            reinterpret_cast<const char*>(
            glGetString(GL_VERSION)));
  kore::Log::getInstance()
    ->write("GLSL version: %s\n",
             reinterpret_cast<const char*>(
             glGetString(GL_SHADING_LANGUAGE_VERSION)));
  kore::Log::getInstance()
    ->write("GLFW version %i.%i.%i\n",
             GLFWmajor, GLFWminor, GLFWrev);
  kore::Log::getInstance()
    ->write("GLEW version: %s\n",
            reinterpret_cast<const char*>(
            glewGetString(GLEW_VERSION)));

  // enable culling and depthtest
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);


  // load shader
  kore::ShaderProgram* simpleShader = new kore::ShaderProgram;
  simpleShader->loadShader("./assets/shader/normalColor.vp",
                          GL_VERTEX_SHADER);
  simpleShader->loadShader("./assets/shader/normalColor.fp",
                          GL_FRAGMENT_SHADER);
  simpleShader->init();
  simpleShader->setName("cooler Shader");

  kore::ShaderProgram* nmShader = new kore::ShaderProgram;
  nmShader->loadShader("./assets/shader/normalmapping.vert", 
                        GL_VERTEX_SHADER);
  nmShader->loadShader("./assets/shader/normalmapping.frag",
                        GL_FRAGMENT_SHADER);
  nmShader->init();
  simpleShader->setName("normal mapping Shader");
  // load resources
  kore::ResourceManager::getInstance()
    //->loadScene("./assets/meshes/TestEnv.dae");
    ->loadScene("./assets/meshes/triangle.dae");

  // texture loading
  kore::Texture* testTexture =
    kore::ResourceManager::getInstance()->
    loadTexture("./assets/textures/Crate.png");

  kore::Texture* stoneTexture =
    kore::ResourceManager::getInstance()->
    loadTexture("./assets/textures/stonewall.png");

  kore::Texture* stoneNormalmap =
    kore::ResourceManager::getInstance()->
    loadTexture("./assets/textures/stonewall_NM_height.png");

  // find camera
  kore::SceneNode* pCameraNode = kore::SceneManager::getInstance()
    ->getSceneNodeByComponent(kore::COMPONENT_CAMERA);
  pCamera = static_cast<kore::Camera*>(
            pCameraNode->getComponent(kore::COMPONENT_CAMERA));

  // find light
  lightNode = kore::SceneManager::getInstance()
    ->getSceneNodeByComponent(kore::COMPONENT_LIGHT);
  kore::LightComponent* pLight = static_cast<kore::LightComponent*>(
    lightNode->getComponent(kore::COMPONENT_LIGHT));

  // select render nodes
  std::vector<kore::SceneNode*> vRenderNodes;
  kore::SceneManager::getInstance()->
                  getSceneNodesByComponent(kore::COMPONENT_MESH, vRenderNodes);


  GLenum drawBuffers[] = {GL_COLOR_ATTACHMENT0};
  kore::FrameBufferStage* backBufferStage = new kore::FrameBufferStage;
  backBufferStage->setFrameBuffer(kore::FrameBuffer::BACKBUFFER,
                                  GL_FRAMEBUFFER,
                                  drawBuffers,
                                  1);

  kore::ShaderProgramPass* shaderProgPass = new kore::ShaderProgramPass;
  //shaderProgPass->setShaderProgram(simpleShader);
  shaderProgPass->setShaderProgram(nmShader);

  // init operations
  for (uint i = 0; i < vRenderNodes.size(); ++i) {
   
      //setUpSimpleRendering(vRenderNodes[i],shaderProgPass,testTexture,pLight);
      setUpNMRendering(vRenderNodes[i],shaderProgPass,stoneTexture,stoneNormalmap,pLight);

  }

  backBufferStage->addProgramPass(shaderProgPass);

  kore::RenderManager::getInstance()->addFramebufferStage(backBufferStage);

 /* std::vector<kore::SceneNode*> vBigCubeNodes;
  kore::SceneManager::getInstance()
    ->getSceneNodesByName("Cube", vBigCubeNodes);
  rotationNode = vBigCubeNodes[0]; */

  glClearColor(1.0f,1.0f,1.0f,1.0f);

  kore::Timer the_timer;
  the_timer.start();
  double time = 0;
  float cameraMoveSpeed = 4.0f;
  
  int oldMouseX = 0;
  int oldMouseY = 0;
  glfwGetMousePos(&oldMouseX,&oldMouseY);

  /*
  //Event-tests
  kore::ResourceManager* resourceManager = kore::ResourceManager::getInstance();

  resourceManager->_fboDeleteEvent.add(backBufferStage, &kore::FrameBufferStage::onFrameBufferDelete);
  resourceManager->_fboDeleteEvent.add(shaderProgPass, &kore::ShaderProgramPass::onFrameBufferDelete);
  resourceManager->_fboDeleteEvent.raiseEvent(NULL);
  resourceManager->_fboDeleteEvent.remove(shaderProgPass, &kore::ShaderProgramPass::onFrameBufferDelete);
  resourceManager->_fboDeleteEvent.raiseEvent(NULL);

  //////////////////////////////////////////////////////////////////////////
  //*/

  

  //initffmpeg("test.mp4",800,600,25);

  // Main loop
  while (running) {
    time = the_timer.timeSinceLastCall();
    kore::SceneManager::getInstance()->update();

    if (glfwGetKey(GLFW_KEY_UP) || glfwGetKey('W')) {
      pCamera->moveForward(cameraMoveSpeed * static_cast<float>(time));
    }

    if (glfwGetKey(GLFW_KEY_DOWN) || glfwGetKey('S')) {
      pCamera->moveForward(-cameraMoveSpeed * static_cast<float>(time));
    }

    if (glfwGetKey(GLFW_KEY_LEFT) || glfwGetKey('A')) {
      pCamera->moveSideways(-cameraMoveSpeed * static_cast<float>(time));
    }

    if (glfwGetKey(GLFW_KEY_RIGHT) || glfwGetKey('D')) {
      pCamera->moveSideways(cameraMoveSpeed * static_cast<float>(time));
    }

    int mouseX = 0;
    int mouseY = 0;
    glfwGetMousePos(&mouseX,&mouseY);

    int mouseMoveX = mouseX - oldMouseX;
    int mouseMoveY = mouseY - oldMouseY;

    if (glfwGetMouseButton(GLFW_MOUSE_BUTTON_1) == GLFW_PRESS ) {
      if (glm::abs(mouseMoveX) > 0 || glm::abs(mouseMoveY) > 0) {
        pCamera->rotateFromMouseMove((float)-mouseMoveX / 5.0f,
                                     (float)-mouseMoveY / 5.0f);
      }
    }

    oldMouseX = mouseX;
    oldMouseY = mouseY;

    if (rotationNode) {
      rotationNode->rotate(90.0f * static_cast<float>(time), glm::vec3(0.0f, 0.0f, 1.0f));
    }
    //lightNode->rotate(-40.0f * static_cast<float>(time), glm::vec3(0.0f, 0.0f, 1.0f), kore::SPACE_WORLD);

    kore::GLerror::gl_ErrorCheckStart();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT |GL_STENCIL_BUFFER_BIT);
    kore::RenderManager::getInstance()->renderFrame();

    
    if (glfwGetKey('R')) {
     //readbuffer();
    }
    glfwSwapBuffers();
    kore::GLerror::gl_ErrorCheckFinish("Main Loop");

    // Check if ESC key was pressed or window was closed
    running = !glfwGetKey(GLFW_KEY_ESC) && glfwGetWindowParam(GLFW_OPENED);
  }

  // Test XML writing
  kore::ResourceManager::getInstance()->saveProject("xmltest.kore");

  // Close window and terminate GLFW
  glfwTerminate();

  // Exit program
  exit(EXIT_SUCCESS);
};
