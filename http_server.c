#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>     //for getopt, fork
#include <string.h>     //for strcat
//for struct evkeyvalq
#include <sys/queue.h>
#include <event.h>
//for http
//#include <evhttp.h>
#include <event2/http.h>
#include <event2/http_struct.h>
#include <event2/http_compat.h>
#include <event2/util.h>
#include <signal.h>
#include <cJSON.h>

#define MYHTTPD_SIGNATURE   "MoCarHttpd v0.1"

//´¦ÀíÄ£¿é
void httpd_handler(struct evhttp_request *req, void *arg) {
    char output[2048] = "\0";
    char tmp[1024];

    //»ñÈ¡¿Í»§¶ËÇëÇóµÄURI(Ê¹ÓÃevhttp_request_uri»òÖ±½Óreq->uri)
    const char *uri;
    uri = evhttp_request_uri(req);
#if 0
    sprintf(tmp, "uri=%s\n", uri);//  /data?cmd=new...
    strcat(output, tmp);
#endif

    sprintf(tmp, "uri=%s\n", req->uri);
    strcat(output, tmp);

    //decoded uri
    char *decoded_uri;
    decoded_uri = evhttp_decode_uri(uri);
    sprintf(tmp, "decoded_uri=%s\n", decoded_uri);// /data?cmd= newFile ...
    strcat(output, tmp);

    //http://127.0.0.1:8080/data?cmd=newFile&fromId=0&count=8

    //½âÎöURIµÄ²ÎÊý(¼´GET·½·¨µÄ²ÎÊý)
    struct evkeyvalq params;//key ---value, key2--- value2//  cmd --- newfile  fromId == 0
    //½«URLÊý¾Ý·â×°³Ékey-value¸ñÊ½,q=value1, s=value2
    evhttp_parse_query(decoded_uri, &params);

    //µÃµ½qËù¶ÔÓ¦µÄvalue
    sprintf(tmp, "username=%s\n", evhttp_find_header(&params, "username"));
    strcat(output, tmp);
    //µÃµ½sËù¶ÔÓ¦µÄvalue
    sprintf(tmp, "passwd=%s\n", evhttp_find_header(&params, "passwd"));
    strcat(output, tmp);

    free(decoded_uri);

    //»ñÈ¡POST·½·¨µÄÊý¾Ý
    char *post_data = (char *) EVBUFFER_DATA(req->input_buffer);
    sprintf(tmp, "post_data=%s\n", post_data);
    strcat(output, tmp);


    /*
       ¾ßÌåµÄ£º¿ÉÒÔ¸ù¾ÝGET/POSTµÄ²ÎÊýÖ´ÐÐÏàÓ¦²Ù×÷£¬È»ºó½«½á¹ûÊä³ö
       ...
     */
    printf("get msg, send : [%s]\n", output);


    /* Êä³öµ½¿Í»§¶Ë */

    //HTTP header
    evhttp_add_header(req->output_headers, "Server", MYHTTPD_SIGNATURE);
    evhttp_add_header(req->output_headers, "Content-Type", "text/plain; charset=UTF-8");
    evhttp_add_header(req->output_headers, "Connection", "close");

    //Êä³öµÄÄÚÈÝ
    struct evbuffer *buf;
    buf = evbuffer_new();
    evbuffer_add_printf(buf, "It works!\n%s\n", output);

    //½«·â×°ºÃµÄevbuffer ·¢ËÍ¸ø¿Í»§¶Ë
    evhttp_send_reply(req, HTTP_OK, "OK", buf);

    evbuffer_free(buf);

}
void reg_handler(struct evhttp_request *req, void *arg) 
{
        printf("get connection!\n");

        //¿¿¿¿¿¿¿¿URI(¿¿evhttp_request_uri¿¿¿req->uri)
        const char *uri;
        uri = evhttp_request_uri(req);
        printf("[uri]=%s\n", uri);


        //¿¿POST¿¿¿¿¿
        size_t post_size = EVBUFFER_LENGTH(req->input_buffer);
        char *request_data = (char *) EVBUFFER_DATA(req->input_buffer);
        char request_data_buf[4096] = {0};
        memcpy(request_data_buf, request_data, post_size);
        printf("[post_data]=\n%s\n", request_data_buf);


        /*
           ¿¿¿¿¿¿¿¿GET/POST¿¿¿¿¿¿¿¿¿¿¿¿¿¿¿¿¿
           ...
           */
        /*
         *      * ==== ¿¿¿¿¿¿¿ ====
         *          https://ip:port/reg [json_data]
         *          {
         *            username: "gailun",
         *             password: "123123",
         *            driver:   "yes/no",
         *            tel:      "13331333333",
         *           email:    "danbing_at@163.cn",
         *          id_card:  "2104041222121211122"
         *       }
         *                                                                       *
         *                                                                            *
         *                                                                                 */

        cJSON *root = cJSON_Parse(request_data_buf);

        printf("username = %s\n", cJSON_GetObjectItem(root, "username")->valuestring);
        printf("password = %s\n", cJSON_GetObjectItem(root, "password")->valuestring);
        printf("driver = %s\n", cJSON_GetObjectItem(root, "driver")->valuestring);
        printf("tel = %s\n", cJSON_GetObjectItem(root, "tel")->valuestring);
        printf("email = %s\n", cJSON_GetObjectItem(root, "email")->valuestring);
        printf("id_card %s\n", cJSON_GetObjectItem(root, "id_card")->valuestring);


        //.¿¿¿¿¿¿¿¿¿¿¿¿

        //¿¿¿¿¿¿¿¿¿¿ ¿¿ok

        /*
           {
           "result" :"ok"
           } 

           {
           "result" :"error",
           "reason" :"...."
           } 


*/
        cJSON *response_root = cJSON_CreateObject();

        cJSON_AddStringToObject(response_root, "result", "ok");




        cJSON_Delete(root);

        char *response_data = NULL;

        response_data = cJSON_Print(response_root);

        /* ¿¿¿¿¿¿ */
        //HTTP header
        evhttp_add_header(req->output_headers, "Server", MYHTTPD_SIGNATURE);
        evhttp_add_header(req->output_headers, "Content-Type", "text/plain; charset=UTF-8");
        evhttp_add_header(req->output_headers, "Connection", "close");

        //¿¿¿¿¿
        struct evbuffer *buf;
        buf = evbuffer_new();
        evbuffer_add_printf(buf, "%s", response_data);

        //¿¿¿¿¿evbuffer ¿¿¿¿¿¿
        evhttp_send_reply(req, HTTP_OK, "OK", buf);

        evbuffer_free(buf);

        printf("[response]:\n");
        printf("%s\n", response_data);

        free(response_data);
        cJSON_Delete(response_root);
}


//´¦ÀíloginµÇÂ½Ä£¿é
void login_handler(struct evhttp_request *req, void *arg) 
{
    printf("get connection!\n");

    //»ñÈ¡¿Í»§¶ËÇëÇóµÄURI(Ê¹ÓÃevhttp_request_uri»òÖ±½Óreq->uri)
    const char *uri;
    uri = evhttp_request_uri(req);
    printf("[uri]=%s\n", uri);


    //»ñÈ¡POST·½·¨µÄÊý¾Ý
    size_t post_size = EVBUFFER_LENGTH(req->input_buffer);
    char *request_data = (char *) EVBUFFER_DATA(req->input_buffer);
    char request_data_buf[4096] = {0};
    memcpy(request_data_buf, request_data, post_size);
    printf("[post_data]=\n%s\n", request_data_buf);


    /*
       ¾ßÌåµÄ£º¿ÉÒÔ¸ù¾ÝGET/POSTµÄ²ÎÊýÖ´ÐÐÏàÓ¦²Ù×÷£¬È»ºó½«½á¹ûÊä³ö
       ...
     */

		cJSON *root = cJSON_Parse(request_data_buf);

   		 printf("username = %s\n", cJSON_GetObjectItem(root, "username")->valuestring);
    		printf("password = %s\n", cJSON_GetObjectItem(root, "password")->valuestring);
   		 printf("driver = %s\n", cJSON_GetObjectItem(root, "driver")->valuestring);
	   	 cJSON *response_root = cJSON_CreateObject();

    		cJSON_AddStringToObject(response_root, "result", "ok");

		    cJSON_Delete(root);

   			 char *response_data = NULL;

   			 response_data = cJSON_Print(response_root);

	

   


    /* Êä³öµ½¿Í»§¶Ë */
    //HTTP header
    evhttp_add_header(req->output_headers, "Server", MYHTTPD_SIGNATURE);
    evhttp_add_header(req->output_headers, "Content-Type", "text/plain; charset=UTF-8");
    evhttp_add_header(req->output_headers, "Connection", "close");

    //Êä³öµÄÄÚÈÝ
    struct evbuffer *buf;
    buf = evbuffer_new();
    evbuffer_add_printf(buf, "%s", response_data);

    //½«·â×°ºÃµÄevbuffer ·¢ËÍ¸ø¿Í»§¶Ë
    evhttp_send_reply(req, HTTP_OK, "OK", buf);

    evbuffer_free(buf);

    printf("[response]:\n");
    printf("%s\n", response_data);

    free(response_data);

	   cJSON_Delete(response_root);

}

void show_help() {
    char *help = "http://localhost:8080\n"
        "-l <ip_addr> interface to listen on, default is 0.0.0.0\n"
        "-p <num>     port number to listen on, default is 1984\n"
        "-d           run as a deamon\n"
        "-t <second>  timeout for a http request, default is 120 seconds\n"
        "-h           print this help and exit\n"
        "\n";
    fprintf(stderr,"%s",help);
}

//µ±Ïò½ø³Ì·¢³öSIGTERM/SIGHUP/SIGINT/SIGQUITµÄÊ±ºò£¬ÖÕÖ¹eventµÄÊÂ¼þÕìÌýÑ­»·
void signal_handler(int sig) {
    switch (sig) {
        case SIGTERM:
        case SIGHUP:
        case SIGQUIT:
        case SIGINT:
            event_loopbreak();  //ÖÕÖ¹ÕìÌýevent_dispatch()µÄÊÂ¼þÕìÌýÑ­»·£¬Ö´ÐÐÖ®ºóµÄ´úÂë
            break;
    }
}

int main(int argc, char *argv[]) {
    //×Ô¶¨ÒåÐÅºÅ´¦Àíº¯Êý
    signal(SIGHUP, signal_handler);
    signal(SIGTERM, signal_handler);
    signal(SIGINT, signal_handler);
    signal(SIGQUIT, signal_handler);

    //Ä¬ÈÏ²ÎÊý
    char *httpd_option_listen = "0.0.0.0";
    int httpd_option_port = 8080;
    int httpd_option_daemon = 0;
    int httpd_option_timeout = 120; //in seconds

    //»ñÈ¡²ÎÊý
    int c;
    while ((c = getopt(argc, argv, "l:p:dt:h")) != -1) {
        switch (c) {
            case 'l' :
                httpd_option_listen = optarg;
                break;
            case 'p' :
                httpd_option_port = atoi(optarg);
                break;
            case 'd' :
                httpd_option_daemon = 1;
                break;
            case 't' :
                httpd_option_timeout = atoi(optarg);
                break;
            case 'h' :
            default :
                show_help();
                exit(EXIT_SUCCESS);
        }
    }

    //ÅÐ¶ÏÊÇ·ñÉèÖÃÁË-d£¬ÒÔdaemonÔËÐÐ
    if (httpd_option_daemon) {
        pid_t pid;
        pid = fork();
        if (pid < 0) {
            perror("fork failed");
            exit(EXIT_FAILURE);
        }
        if (pid > 0) {
            //Éú³É×Ó½ø³Ì³É¹¦£¬ÍË³ö¸¸½ø³Ì
            exit(EXIT_SUCCESS);
        }
    }


    /* Ê¹ÓÃlibevent´´½¨HTTP Server */

    //³õÊ¼»¯event API
    event_init();

    //´´½¨Ò»¸öhttp server
    struct evhttp *httpd;

    httpd = evhttp_start(httpd_option_listen, httpd_option_port);

    evhttp_set_timeout(httpd, httpd_option_timeout);

    //Ö¸¶¨generic callback
    // evhttp_set_gencb(httpd, httpd_handler, NULL);
    //Ò²¿ÉÒÔÎªÌØ¶¨µÄURIÖ¸¶¨callback
    evhttp_set_cb(httpd, "/", httpd_handler, NULL);
    evhttp_set_cb(httpd, "/login", login_handler,NULL);
    evhttp_set_cb(httpd, "/reg", reg_handler,NULL);

    //Ñ­»·´¦Àíevents
    event_dispatch();

    evhttp_free(httpd);

    return 0;
}
