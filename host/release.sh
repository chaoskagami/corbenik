#!/bin/bash

cd out

rm -f release.zip sha512sums

zip -r9 release.zip *
sha512sum -b `find . -type f` > sha512sums

cd ..
