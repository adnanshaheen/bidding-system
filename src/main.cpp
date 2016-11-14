
/* Headers */
#include "support.h"
#include "log.h"
#include "manager.h"

/* options structure */
struct _opts {
	int debug;
	unsigned int bidders;
	unsigned short port;
} opts;

/*
 * signal handler for Ctrl+C, and Segmentation Fault
 */
void signal_handler(int arg __attribute__((unused)))
{
	/* TODO */
}

/*
 * Print usage of project
 */
void Usage()
{
	printf("Usage: project0 [options]\n"
		"\n"
		"    Bidding process\n"
		"\n"
		"    -b, --bidders NUMBER    Set number of bidders\n"
		"    -p, --port NUMBER       Set port number for manager\n"
#ifdef DEBUG
		"    -d, --debug             Show debugging information\n"
#endif // DEBUG
		"\n");
}

/*
 * Parse input paramters
 */
int parse_options(int argc, char **argv)
{
	const char *pOpt = "-b:p:d";
	const struct option cOpt[] = {
#ifdef DEBUG
		{ "debug",	no_argument,		NULL, 'd' },		/* show debugging messages */
#endif
		{ "bidders",	required_argument,	NULL, 'b' },	/* Set number of bidders */
		{ "port",	required_argument,	NULL, 'p' },		/* Set port number for manager */
		{ NULL, 0, NULL, 0 }
	};

	int res = 0;
	int help = 0;
	int c = 0;
	memset(&opts, 0, sizeof(opts));
	while ((c = getopt_long(argc, argv, pOpt, cOpt, NULL)) != -1) {
		switch (c) {
#ifdef DEBUG
		case 'd':
			opts.debug = 1;
			break;
#endif // DEBUG
		case 'b':
			if (opts.bidders == 0)
				opts.bidders = atoll(argv[optind - 1]);
			else
				res = 1;
			break;
		case 'p':
			if (opts.port == 0)
				opts.port = atoll(argv[optind - 1]);
			else
				res = 1;
			break;
		default:
			perr_printf("Invalid arguments");
			Usage();
			help = 1;
			break;
		}
	}

	return (!res && !help);
}

/*
 * main program
 */
int main(int argc, char* argv[])
{
	/* Set SIGINT handler. */
	if (signal(SIGINT, signal_handler) == SIG_ERR) {
		perr_printf("Failed to set SIGINT handler");
		return 1;
	}
	/* Set SIGTERM handler. */
	if (signal(SIGTERM, signal_handler) == SIG_ERR) {
		perr_printf("Failed to set SIGTERM handler");
		return 1;
	}

	/* parase options */
	if (!parse_options(argc, argv)) {
		return 1;
	}

	/* display manager pid */
	pid_t nPid = getpid();
	log_message("Manager PID is %d.", nPid);

	int nBidders = opts.bidders;
	int nPort = opts.port;

	/* check for valid bidders */
	while (nBidders == 0) {

		/* FIXME: Check for non integer values */
		log_message("Enter number of bidders");
		scanf("%d", &nBidders);
		if (nBidders < DEFAULT_BIDDERS || nBidders > MAX_BIDDERS) {
			log_message("Please enter between %d and %d", DEFAULT_BIDDERS, MAX_BIDDERS);
			nBidders = 0;
		}
	}

	CManager cManager(nBidders, nPort);		/* Create manager */
	cManager.Start();						/* initialize bidding process */
	
	return 0;
}
