FROM gcc:latest
COPY . /usr/src/sqlite3_string_distance
WORKDIR /usr/src/sqlite3_string_distance
#RUN gcc -o myapp main.c
RUN make
CMD ["./myapp"]
