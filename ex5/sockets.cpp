#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstring>
#include <netdb.h>
#include <unistd.h>

#define MAXHOSTNAME 8; // TODO what number to put here



int establish(unsigned short portnum) {
    char myname[MAXHOSTNAME + 1];
    int s;
    struct sockaddr_in sa{};
    struct hostent *hp;
    //hostnet initialization
    gethostname(myname, MAXHOSTNAME);
    hp = gethostbyname(myname);
    if (hp == nullptr)
        return(-1);
    //sockaddrr_in initlization
    memset(&sa, 0, sizeof(struct sockaddr_in));
    sa.sin_family = hp->h_addrtype;
    /* this is our host address */
    memcpy(&sa.sin_addr, hp->h_addr, hp->h_length);
    /* this is our port number */
    sa.sin_port= htons(portnum);
    /* create socket */
    if ((s= socket(AF_INET, SOCK_STREAM, 0)) <
        0)
        return(-1);
    if (bind(s , (struct sockaddr *)&sa , sizeof(struct
            sockaddr_in)) < 0) {
        close(s);
        return(-1);
    }
    listen(s, 3); /* max # of queued connects */
    return(s);
}

int get_connection(int s) {
    int t; /* socket of connection */
    if ((t = accept(s,nullptr,nullptr)) < 0)
        return -1;
    return t;
}

int call_socket(char *hostname, unsigned short portnum) {
    struct sockaddr_in sa{};
    struct hostent *hp;
    int s;
    if ((hp= gethostbyname (hostname)) == NULL)
    {
        return(-1);
    }
    memset(&sa,0,sizeof(sa));
    memcpy((char *)&sa.sin_addr, hp->h_addr, hp->h_length);
    sa.sin_family = hp->h_addrtype;
    sa.sin_port = htons((u_short)portnum);
    if ((s = socket(hp->h_addrtype,
                    SOCK_STREAM,0)) < 0) {
        return(-1);
    }
    if (connect(s, (struct sockaddr *)&sa , sizeof(sa))
        < 0) {
        close(s);
        return(-1);
    }
    return(s);
}

int read_data(int s, char *buf, int n) {
    int bytesTotalCount; /* counts bytes read */
    int bytesThisRound; /* bytes read this pass */
    bytesTotalCount= 0; bytesThisRound= 0;
    while (bytesTotalCount < n) { /* loop until full buffer */
        bytesThisRound = read(s, buf, n-bytesTotalCount))
        if ((br > 0) {
            bcount += br;
            buf += br;
        }
        if (br < 1) {
            return(-1);
        }
    }
    return(bytesTotalCount);
}



int main(int argc, char* argv[]) {
    struct sockaddr_in my_addr{};
    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(3490);
    inet_aton("10.12.110.57",&(my_addr.sin_addr));
    memset(&(my_addr.sin_zero), '\0', 8);
}

