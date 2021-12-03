#!/bin/bash

kill $(ps aux | grep '[m]emcached' | awk '{print $2}');
