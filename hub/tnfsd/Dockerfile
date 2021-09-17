FROM ubuntu:latest
MAINTAINER Thomas Cherryhomes <thom.cherryhomes@gmail.com>
EXPOSE 16384
VOLUME /data
ADD bin/tnfsd /bin
RUN chmod +x /bin/tnfsd
ENTRYPOINT ["/bin/tnfsd","/data"]
