FROM ubuntu:16.04@sha256:a3785f78ab8547ae2710c89e627783cfa7ee7824d3468cae6835c9f4eae23ff7

RUN apt-get update
RUN apt-get install -y netcat wget git gcc g++ make cmake libboost-all-dev curl libssl-dev libcurl4-openssl-dev

WORKDIR /opt/dji/flightlog-decoder

COPY . .

WORKDIR /opt/dji/flightlog-decoder/dji-flightrecord-kit/build/Ubuntu/FlightLogDecoderApi
RUN sh generate.sh

ENTRYPOINT ["./FlightLogDecoderApi"]
