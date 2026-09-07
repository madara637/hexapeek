# diskpeek

Small command line tool for Linux that scans a directory and shows you
what's taking up the most space

## build

Needs g++ with C++17 support (basically any semi-recent version).

```
make
```

This spits out a `diskpeek` binary in the same folder.

## usage

```
./diskpeek <directory>
./diskpeek /home/me/Downloads
./diskpeek . -n 10
```

- `-n <num>` : how many results to print (default 20)
- `--help` : prints usage


## license

do whatever you want with it
