#!/bin/bash
thorq --add --mode snucl --nodes 5 --device gpu ./snucl_swaptions -ns 128 -sm 1280000 -gs 64 -ps 16
