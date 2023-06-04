FROM gcc:12.3-bullseye
RUN groupadd -g 1000 --system server && useradd -u 1000 -g server --system server
WORKDIR /home/server
# COPY . . (use in production)
CMD ["./make.sh"]