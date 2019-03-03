/* This is an example application source code using the multi interface
 * to do a multipart formpost without "blocking". */
#include <stdio.h>  
#include <string.h>  

#ifndef WIN32
#include <sys/time.h>
#else
#pragma comment(lib, "Ws2_32.lib")
#endif

#include <curl/curl.h>  

int main(int argc ,char** argv)
{
	CURL *curl;

	CURLM *multi_handle;
	int still_running;

	struct curl_httppost *post = NULL;
	struct curl_httppost *last = NULL;
	struct curl_slist *headerlist = NULL;
	static const char buf[] = "Expect:";

	const char* url = "47.98.253.51/upload?path=";

	if (argc > 2) {
		url = argv[1];
	}

	/* Fill in the file upload field. This makes libcurl load data from
	   the given file name when curl_easy_perform() is called. */
	curl_formadd(&post,
		&last,
		CURLFORM_PTRNAME, "file",
		CURLFORM_FILE, "d://H264//slamtv60.264",
		CURLFORM_FILENAME, "slamtv60.264",
		CURLFORM_END);// form-data key(file) "./test.jpg"为文件路径  "slamtv60.264" 为文件上传时文件名


	curl_formadd(&post,
		&last,
		CURLFORM_PTRNAME, "file",
		CURLFORM_FILE, "d://H264//udpraw.bin",
		CURLFORM_FILENAME, "udpraw.bin",
		CURLFORM_END);// form-data key(file) "./test.jpg"为文件路径  "slamtv60.264" 为文件上传时文件名

	curl_formadd(&post,
		&last,
		CURLFORM_PTRNAME, "file",
		CURLFORM_FILE, "d://H264//girl.mp4",
		CURLFORM_FILENAME, "girl.mp4",
		CURLFORM_END);// form-data key(file) "./test.jpg"为文件路径  "slamtv60.264" 为文件上传时文件名


	curl = curl_easy_init();
	multi_handle = curl_multi_init();

	/* initalize custom header list (stating that Expect: 100-continue is not
	   wanted */
	headerlist = curl_slist_append(headerlist, buf);
	if (curl && multi_handle) {

		/* what URL that receives this POST */
		curl_easy_setopt(curl, CURLOPT_URL, url);
		curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerlist);
		curl_easy_setopt(curl, CURLOPT_HTTPPOST, post);

		curl_multi_add_handle(multi_handle, curl);

		curl_multi_perform(multi_handle, &still_running);

		do {
			struct timeval timeout;
			int rc; /* select() return code */

			fd_set fdread;
			fd_set fdwrite;
			fd_set fdexcep;
			int maxfd = -1;

			long curl_timeo = -1;

			FD_ZERO(&fdread);
			FD_ZERO(&fdwrite);
			FD_ZERO(&fdexcep);

			/* set a suitable timeout to play around with */
			timeout.tv_sec = 1;
			timeout.tv_usec = 0;

			curl_multi_timeout(multi_handle, &curl_timeo);
			if (curl_timeo >= 0) {
				timeout.tv_sec = curl_timeo / 1000;
				if (timeout.tv_sec > 1)
					timeout.tv_sec = 1;
				else
					timeout.tv_usec = (curl_timeo % 1000) * 1000;
			}

			/* get file descriptors from the transfers */
			curl_multi_fdset(multi_handle, &fdread, &fdwrite, &fdexcep, &maxfd);

			/* In a real-world program you OF COURSE check the return code of the
			   function calls.  On success, the value of maxfd is guaranteed to be
			   greater or equal than -1.  We call select(maxfd + 1, ...), specially in
			   case of (maxfd == -1), we call select(0, ...), which is basically equal
			   to sleep. */

			rc = select(maxfd + 1, &fdread, &fdwrite, &fdexcep, &timeout);

			switch (rc) {
			case -1:
				/* select error */
				break;
			case 0:
			default:
				/* timeout or readable/writable sockets */
				printf("perform!\n");
				curl_multi_perform(multi_handle, &still_running);
				printf("running: %d!\n", still_running);
				break;
			}
		} while (still_running);

		curl_multi_cleanup(multi_handle);

		/* always cleanup */
		curl_easy_cleanup(curl);

		/* then cleanup the formpost chain */
		curl_formfree(post);

		/* free slist */
		curl_slist_free_all(headerlist);
	}
	return 0;
}