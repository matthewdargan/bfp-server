// Copyright (c) 2023, Matthew R. Muller, Matthew P. Dargan.
// SPDX-License-Identifier: BSD-3-Clause

#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <netinet/in.h>

#include <ctype.h>
#include <dirent.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define PORT 8039
#define MAX_CONNS 100
#define MAX_BUFFER_SIZE 1024

void handle_client(int client_fd)
{
	char buf[MAX_BUFFER_SIZE], clean_buf[MAX_BUFFER_SIZE],
		out[MAX_BUFFER_SIZE];
	DIR *dir;
	struct dirent *dp;
	struct stat st;
	ssize_t nr, rcount;
	int end, fd, start;
	const char nul = '\0';

	while ((rcount = read(client_fd, buf, sizeof(buf))) > 0) {
		buf[rcount] = '\0';

		// Trim leading and trailing whitespace
		start = 0;
		end = strlen(buf) - 1;
		while (start <= end && isspace(buf[start]))
			start++;
		while (end >= start && isspace(buf[end]))
			end--;

		// If entire string is whitespace, skip it
		if (start > end)
			continue;
		strncpy(clean_buf, buf + start, end - start + 1);
		clean_buf[end - start + 1] = '\0';

		// Send null character to indicate an invalid request
		if (stat(clean_buf, &st) == -1) {
			write(client_fd, &nul, 1);
			continue;
		}

		if (S_ISDIR(st.st_mode)) {
			if ((dir = opendir(clean_buf)) != NULL) {
				// Send the contents of the specified directory
				while ((dp = readdir(dir)) != NULL) {
					snprintf(out, sizeof(out), "%s\n",
						 dp->d_name);
					write(client_fd, out, strlen(out));
				}
				closedir(dir);
			}
		} else if ((fd = open(clean_buf, O_RDONLY)) != -1) {
			// Send the contents of the file
			while ((nr = read(fd, out, sizeof(out))) > 0)
				write(client_fd, out, nr);
			close(fd);
		}
	}
	close(client_fd);
}

int main(void)
{
	struct sockaddr_in addr, clientaddr;
	int client_fd, fd;
	socklen_t len = sizeof(clientaddr);

	if ((fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
		return 1;
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(PORT);
	if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
		close(fd);
		return 1;
	}
	if (listen(fd, MAX_CONNS) == -1) {
		close(fd);
		return 1;
	}

	while (1) {
		client_fd = accept(fd, (struct sockaddr *)&clientaddr, &len);
		if (client_fd == -1)
			continue;
		handle_client(client_fd);
	}
	close(fd);
	return 0;
}
