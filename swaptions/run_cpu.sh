#!/bin/bash
thorq --add --mode single --device gpu ./cpu_swaptions -ns 128 -sm 1280000 -gs 64

