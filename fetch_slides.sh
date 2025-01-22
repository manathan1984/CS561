#!/bin/bash

set -x
cp ~/Dropbox/Professional/Professorship/W-Lab/CS561-Spring2025/slides-PDFs/*.pdf ./slides
# cp ~/Dropbox/Professional/Professorship/0_CLASSES/CS561-Spring2025/Slides-PDF/*.pdf ./slides
# cp  ~/Repos/cs460-660-materials/_cs660-slides-pdf/*.pdf slides/

if [ $# -ne 0 ]; then
    echo "Only copying, no pushing."
    exit
fi

git pull

if [ $? -ne 0 ]; then
    echo "Please ensure manually that pull resulted in a sane repo. Exiting ..."
    exit
fi
git add slides/*
git commit -m "CS660: adding new slides"
git push
set +x