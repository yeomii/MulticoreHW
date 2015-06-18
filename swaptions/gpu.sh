#!/bin/bash
thorq --add --mode single --device gpu ./gpu_swaptions -ns 128 -sm 1000000 -gs 64

