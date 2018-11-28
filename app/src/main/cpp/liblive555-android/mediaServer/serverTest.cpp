//
// Created by kingMT on 2018/11/28.
//

#include <jni.h>
#include <BasicUsageEnvironment.hh>
#include <string>
#include "DynamicRTSPServer.hh"
#include "version.hh"
#include "../../MG_log.h"


extern "C" JNIEXPORT void
JNICALL
Java_google_github_RtspServer_RtspServer_startServer() {
    // Begin by setting up our usage environment:
    TaskScheduler* scheduler = BasicTaskScheduler::createNew();
    UsageEnvironment* env = BasicUsageEnvironment::createNew(*scheduler);

    UserAuthenticationDatabase* authDB = NULL;
#ifdef ACCESS_CONTROL
    // To implement client access control to the RTSP server, do the following:
  authDB = new UserAuthenticationDatabase;
  authDB->addUserRecord("username1", "password1"); // replace these with real strings
  // Repeat the above with each <username>, <password> that you wish to allow
  // access to the server.
#endif

    // Create the RTSP server.  Try first with the default port number (554),
    // and then with the alternative port number (8554):
    RTSPServer* rtspServer;
    portNumBits rtspServerPortNum = 554;
    rtspServer = DynamicRTSPServer::createNew(*env, rtspServerPortNum, authDB);
    if (rtspServer == NULL) {
        rtspServerPortNum = 8554;
        rtspServer = DynamicRTSPServer::createNew(*env, rtspServerPortNum, authDB);
    }
    if (rtspServer == NULL) {
        *env << "Failed to create RTSP server: " << env->getResultMsg() << "\n";
        return ;
    }

    LOGD("LIVE555 Media Server\n") ;
    *env << "\tversion " << MEDIA_SERVER_VERSION_STRING
         << " (LIVE555 Streaming Media library version "
         << LIVEMEDIA_LIBRARY_VERSION_STRING << ").\n";

    char* urlPrefix = rtspServer->rtspURLPrefix();
    std::string info;
    info += "\n\tPlay streams from this server using the URL:\n\t";
    info += urlPrefix;
    info += "<filename>\nwhere <filename> is a file present in the current directory.\n";
    info += "Each file's type is inferred from its name suffix:\n";
    info += "\t\".264\" => a H.264 Video Elementary Stream file\n";
    info += "\t\".265\" => a H.265 Video Elementary Stream file\n";
    info += "\t\".aac\" => an AAC Audio (ADTS format) file\n";
    info += "\t\".ac3\" => an AC-3 Audio file\n";
    info += "\t\".amr\" => an AMR Audio file\n";
    info += "\t\".dv\" => a DV Video file\n";
    info += "\t\".m4e\" => a MPEG-4 Video Elementary Stream file\n";
    info += "\t\".mkv\" => a Matroska audio+video+(optional)subtitles file\n";
    info += "\t\".mp3\" => a MPEG-1 or 2 Audio file\n";
    info += "\t\".mpg\" => a MPEG-1 or 2 Program Stream (audio+video) file\n";
    info += "\t\".ogg\" or \".ogv\" or \".opus\" => an Ogg audio and/or video file\n";
    info += "\t\".ts\" => a MPEG Transport Stream file\n";
    info += "\t\t(a \".tsx\" index file - if present - provides server 'trick play' support)\n";
    info += "\t\".vob\" => a VOB (MPEG-2 video with AC-3 audio) file\n";
    info += "\t\".wav\" => a WAV Audio file\n";
    info += "\t\".webm\" => a WebM audio(Vorbis)+video(VP8) file\n";
    info += "See http://www.live555.com/mediaServer/ for additional documentation.\n";

    // Also, attempt to create a HTTP server for RTSP-over-HTTP tunneling.
    // Try first with the default HTTP port (80), and then with the alternative HTTP
    // port numbers (8000 and 8080).

    if (rtspServer->setUpTunnelingOverHTTP(80) || rtspServer->setUpTunnelingOverHTTP(8000) || rtspServer->setUpTunnelingOverHTTP(8080)) {
        info += "(We use port ";
        info += std::to_string(rtspServer->httpServerPortNum());
        info += " for optional RTSP-over-HTTP tunneling, or for HTTP live streaming (for indexed Transport Stream files only).)\n";
    } else {
        info += "(RTSP-over-HTTP tunneling is not available.)\n";
    }

    LOGI("%s", info.c_str());
    env->taskScheduler().doEventLoop(); // does not return
}