#!/bin/bash
rsync -rvt * pmkelly,adelaideos@web.sourceforge.net:htdocs/ --exclude update-web.sh
