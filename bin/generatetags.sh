#!/bin/sh

cd ../..
git clone https://github.com/mhwkb/mhwkb.github.io
cd mhwkb.github.io

echo -e "List of Existing Tags\n" >> tags.txt
echo -e "If you need suggestions for tags, this is a good list for reference.\n" >> tags.txt

DATE=`date +%Y-%m-%d`
echo -e "Last updated $DATE\n" >> tags.txt

ls >> tags.txt

sed -i 's/.html//g' tags.txt

sed -i '/LICENSE/d' tags.txt
sed -i '/README.md/d' tags.txt
sed -i '/mhwkb_style.css/d' tags.txt
sed -i '/tags.txt/d' tags.txt

exit 0;