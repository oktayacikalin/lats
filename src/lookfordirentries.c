#include <sys/types.h>
#include <linux/dirent.h>
#include <sys/fcntl.h>
#include <unistd.h>
#include <stdio.h>

char buf[2048];

void
main(int argc, char *argv[])
{
	int fd;
	struct dirent *dp;
	int n;
	off_t off;

	if (argc != 2) {
		fprintf(stderr, "usage: %s directory_name\n", argv[0]);
		exit(1);
}
if ((fd = open(argv[1], O_RDONLY)) < 0) {
		perror("Cannot open directory");
		exit(1);
}
while ((n = getdents(fd, (struct dirent *)buf, 2048)) > 0) {
		printf("getdents read %d bytes\n", n);
		printf("offset = %d\n", lseek(fd, 0, SEEK_CUR));
		dp = (struct dirent *)buf;
		while (n > 0) {
			printf("\t%s\t%d\t%d\n", dp->d_name, dp->d_reclen,
			    dp->d_off);
			n -= dp->d_reclen;
			dp = (struct dirent *)((char *)dp + dp->d_reclen);
}
	}
if (n < 0) {
		perror("getdents failed");
		exit(1);
} exit(0);
}

