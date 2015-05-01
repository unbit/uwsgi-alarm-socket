#include <uwsgi.h>

/*

	spawn a thread for every socket to monitor, each thread
	will raise an alarm whenever a SOCK_STREAM connection fails

*/

extern struct uwsgi_server uwsgi;

static struct alarm_socket_opts {
	struct uwsgi_string_list *sockets;
	int freq;
	int timeout;
} alarm_socket_opts;

static struct uwsgi_option alarm_socket_options[] = {
	{"alarm-socket", required_argument, 0, "raise an alarm when the specified socket is not reachable", uwsgi_opt_add_string_list, &alarm_socket_opts.sockets, UWSGI_OPT_MASTER},
	{"alarm-socket-freq", required_argument, 0, "set alarm-socket frequency (default 30 seconds)", uwsgi_opt_set_int, &alarm_socket_opts.freq, 0},
	{"alarm-socket-timeout", required_argument, 0, "set alarm-socket connection timeout (default 5 seconds)", uwsgi_opt_set_int, &alarm_socket_opts.timeout, 0},
	UWSGI_END_OF_OPTIONS
};

static void *alarm_socket_loop(void *arg) {

	char *alarm_name = (char *) arg;
	char *socket_name = strchr(alarm_name, ' ');
	// overengineering !!!
	if (!socket_name) {
		uwsgi_log_verbose("fatal error alarm_socket_loop()\n");
		exit(1);
	}
	// fix socket name
	*socket_name = 0;
	socket_name++;
	
	for(;;) {
		// wait for the alarm subsystem to be fully ready
		// (simple spinner)
		if (!uwsgi.alarm_thread) {
			sleep(1);
			continue;
		}
		// if we are here the alarm subsystem is ready
		int fd = uwsgi_connect(socket_name, alarm_socket_opts.timeout, 0);
		if (fd < 0) {
			char buf[1024];
                        int ret = snprintf(buf, 1024, "unable to connect to \"%s\": %s", socket_name, strerror(errno));
                        if (ret > 0 && ret < 1024) {
				uwsgi_alarm_trigger(alarm_name, buf, ret);
			}
		}
		else {
			close(fd);
		}
		sleep(alarm_socket_opts.freq);
	}
}

static int alarm_socket_setup() {
	if (!alarm_socket_opts.timeout) alarm_socket_opts.timeout = 5;
	if (!alarm_socket_opts.freq) alarm_socket_opts.freq = 30;

	struct uwsgi_string_list *usl = NULL;
	uwsgi_foreach(usl, alarm_socket_opts.sockets) {
		char *space = strchr(usl->value, ' ');
		if (!space) {
			uwsgi_log("[alarm-socket] invalid syntax, must be <alarm> <socket>\n");
			exit(1);
		}
		pthread_t t;
		if (pthread_create(&t, NULL, alarm_socket_loop, uwsgi_str(usl->value))) {
			uwsgi_error("alarm_socket_setup()/pthread_create()");
			exit(1);
		}
	}
	return 0;
}


struct uwsgi_plugin alarm_socket_plugin = {
	.name = "alarm_socket",
	.options = alarm_socket_options,
	.init = alarm_socket_setup,
};
