set -e

docker build . -t nm_run
docker run -v `pwd`:/app -it nm_run bash
