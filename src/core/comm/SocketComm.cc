#include <string>
#include <errno.h>
#include <signal.h>
#include <poll.h>

#include "SocketComm.hpp"

namespace fail {

void SocketComm::init()
{
	// It's usually much easier to handle the error on write(), than to do
	// anything intelligent in a SIGPIPE handler.
	signal(SIGPIPE, SIG_IGN);
}

bool SocketComm::sendMsg(int sockfd, google::protobuf::Message& msg)
{
	int size = htonl(msg.ByteSize());
	std::string buf;
	if (safe_write(sockfd, &size, sizeof(size)) == -1
			|| !msg.SerializeToString(&buf)
			|| safe_write(sockfd, buf.c_str(), buf.size()) == -1) {
		return false;
	}
	return true;
}

bool SocketComm::rcvMsg(int sockfd, google::protobuf::Message& msg)
{
	char *buf;
	int bufsiz;
	if ((buf = getBuf(sockfd, &bufsiz))) {
		std::string st(buf, bufsiz);
		delete [] buf;
		return msg.ParseFromString(st);
	}
	return false;
}

bool SocketComm::dropMsg(int sockfd)
{
	char *buf;
	int bufsiz;
	if ((buf = getBuf(sockfd, &bufsiz))) {
		delete [] buf;
		return true;
	}
	return false;
}

char * SocketComm::getBuf(int sockfd, int *size)
{
	char *buf;
	if (safe_read(sockfd, size, sizeof(int)) == -1) {
		return 0;
	}
	*size = ntohl(*size);
	buf = new char[*size];
	if (safe_read(sockfd, buf, *size) == -1) {
		delete [] buf;
		return 0;
	}
	return buf;
}

int SocketComm::timedAccept(int sockfd, struct sockaddr *addr, socklen_t *addrlen, int timeout)
{
	struct pollfd pfd = {sockfd, POLLIN, 0};
	int ret = poll(&pfd, 1, timeout);
	if (ret > 0) {
		return accept(sockfd, addr, addrlen);
	}
	errno = EWOULDBLOCK;
	return -1;
}

ssize_t SocketComm::safe_write(int fd, const void *buf, size_t count)
{
	ssize_t ret;
	const char *cbuf = (const char *) buf;
	do {
		ret = write(fd, cbuf, count);
		if (ret == -1) {
			if (errno == EINTR) {
				continue;
			}
			return -1;
		}
		count -= ret;
		cbuf += ret;
	} while (count);
	return cbuf - (const char *)buf;
}

ssize_t SocketComm::safe_read(int fd, void *buf, size_t count)
{
	ssize_t ret;
	char *cbuf = (char *) buf;
	do {
		ret = read(fd, cbuf, count);
		if (ret == -1) {
			if (errno == EINTR) {
				continue;
			}
			return -1;
		} else if (ret == 0) {
			// this deliberately deviates from read(2)
			return -1;
		}
		count -= ret;
		cbuf += ret;
	} while (count);
	return cbuf - (const char *) buf;
}

} // end-of-namespace: fail
