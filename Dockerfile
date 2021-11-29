FROM gcc:latest
COPY . /usr/src/sqlite3_distance_multibase
WORKDIR /usr/src/sqlite3_distance_multibase
RUN make
CMD ["./myapp"]
