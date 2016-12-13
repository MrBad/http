#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>

int demonize(const char *pid_file) 
{
	pid_t pid;
	int fd;
	char buf[16];
	struct stat st;
	

	fd = open(pid_file, O_RDONLY);
	if(fd != -1) {
		read(fd, buf, sizeof(buf));
		close(fd);
		pid = atoi(buf);
		snprintf(buf, sizeof(buf), "/proc/%d", pid);
		if(stat(buf, &st) == 0 && S_ISDIR(st.st_mode)) {
			fprintf(stdout, "Already running, as pid %d, exiting\n", pid);
			exit(1);
		}
	}

	fprintf(stdout, "Falling into the background as pid: ");
	
	pid = fork();
	if(pid < 0) {
		perror("fork");
		return -1;
	} else if(pid == 0) {
		return 0;
	} else {
		fprintf(stdout, "%d\n", pid);

		fd = open(pid_file, O_WRONLY | O_CREAT, 0600);
		if(fd < 0) {
			perror("open");
			return -1;
		}
		snprintf(buf, sizeof(buf), "%d", pid); 
		write(fd, buf, strlen(buf));
		close(fd);
		exit (0); // bye father
	}
	return 0;
}
