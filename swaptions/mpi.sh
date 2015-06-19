#!/bin/bash
thorq --add --mode mpi --nodes 4 --slots 1 --device gpu ./mpi_swaptions -ns 128 -sm 1280000 -gs 64 -ps 16
