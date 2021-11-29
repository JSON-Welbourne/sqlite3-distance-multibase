FROM gcc:latest
COPY . /usr/src/sqlite3_distance_multibase
WORKDIR /usr/src/sqlite3_distance_multibase
#RUN gcc -o myapp main.c
RUN make
CMD ["./myapp"]
