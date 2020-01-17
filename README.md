# ThingsDB

## TODO / Road map

- [ ] Update tests
    - [ ] More subscribe tests
    - [ ] More Type tests
    - [ ] More Wrap tests
    - [ ] Complete ThingsDB functions test
- [ ] Update documentation
- [x] Documentation testing
- [ ] Start project **ThingsBook** for a beginner guide on how to work with ThingsDB
- [x] Accept environment variable and make config file optional
    - When both a config file and environment variable, the latter should win
- [ ] Syntax Highlighting
    - [x] Pygments (Python)
    - [x] Chroma  (Go, first make Pygments, it should be easy to convert to Chroma)
    - [ ] Monoca editor
    - [ ] VSCode (related to Monaco editor)
    - [ ] Ace editor
- [x] set_type, for configuring an empty type without instances
- [ ] thing.watch()/thing.unwatch() function calls.
- [ ] sets add/remove

## Plans and Ideas for the Future
- [ ] Big number support?
- [ ] support for things with any "raw" keys?

## Get status

```
wget -q -O - http://node.local:8080/status
```

## Special thanks to:

 - [Fast Validate UTF-8](https://github.com/lemire/fastvalidate-utf-8)

## Fonts:

https://fonts.adobe.com/fonts/keraleeyam

## Install pcre2, yajl, libuv from apt:
```
sudo apt install -y libpcre2-dev libyajl-dev libuv1-dev
```

## Install libcleri
```
git clone https://github.com/transceptor-technology/libcleri.git
cd Release
make clean
make
sudo make install
```

## Run integration tests
```
docker build -t thingsdb/itest -f itest/Dockerfile .
docker run thingsdb/itest:latest
```
