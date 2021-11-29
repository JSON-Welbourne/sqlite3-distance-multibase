# sqlite3_distance_multibase
How to use:

sudo docker run --rm -v "$PWD":/usr/src/myapp -w /usr/src/myapp gcc:latest make

OR

sudo make

    cc -shared -fPIC -o sqlite3_distance.so sqlite3_distance.c -lsqlite3

sqlite3

sqlite> SELECT load_extension('./sqlite3_distance.so', 'distance_init');

Loaded sqlite3_distance!

    any string or integer representing an integer between 2 and 255 (examples: 2, '2', 10, '16', '64'...)
    32_RFC4648
    32_Crockford
    32_Hex
    32_Geohash
    32_WordSafe
    z_base_32
    ascii85
    85_RFC1924
    basE91
    
Usage:

    SELECT distance({type},{base},{stringA},{stringB});
    
Print Tables:

    SELECT distance({type},{base},NULL,NULL);
    
Examples:

    SELECT distance('hamming',2,'0110','1001');
    SELECT distance('hamming','ascii85','Aa123','Bb456');
    SELECT distance('hamming','64','Aa123','Bb456');
    SELECT distance('hamming','64',NULL,NULL);
    
    
sqlite> SELECT distance('hamming','ascii85',NULL,NULL););

ascii85:
    
    0:      0    0    0    0    0    0    0    0
    8:      0    0    0    0    0    0    0    0
    16:     0    0    0    0    0    0    0    0
    24:     0    0    0    0    0    0    0    0
    32:     0    0    1    2    3    4    5    6
    40:     7    8    9   10   11   12   13   14
    48:    15   16   17   18   19   20   21   22
    56:    23   24   25   26   27   28   29   30
    64:    31   32   33   34   35   36   37   38
    72:    39   40   41   42   43   44   45   46
    80:    47   48   49   50   51   52   53   54
    88:    55   56   57   58   59   60   61   62
    96:    63   64   65   66   67   68   69   70
    104:   71   72   73   74   75   76   77   78
    112:   79   80   81   82   83   84    0    0
    120:    0    0    0    0    0    0    0    0
    128:    0    0    0    0    0    0    0    0
    136:    0    0    0    0    0    0    0    0
    144:    0    0    0    0    0    0    0    0
    152:    0    0    0    0    0    0    0    0
    160:    0    0    0    0    0    0    0    0
    168:    0    0    0    0    0    0    0    0
    176:    0    0    0    0    0    0    0    0
    184:    0    0    0    0    0    0    0    0
    192:    0    0    0    0    0    0    0    0
    200:    0    0    0    0    0    0    0    0
    208:    0    0    0    0    0    0    0    0
    216:    0    0    0    0    0    0    0    0
    224:    0    0    0    0    0    0    0    0
    232:    0    0    0    0    0    0    0    0
    240:    0    0    0    0    0    0    0    0
    248:    0    0    0    0    0    0    0    0
 

sqlite> SELECT distance('hamming','ascii85','Aa123','Bb456');

    9
