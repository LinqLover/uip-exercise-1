#!/usr/bin/python3

import sys

from lxml import html


def main(input, output):

    with open(input, 'r') as stream:
        doc = html.fromstring(stream.read().encode())

    texts = [elem.text for elem in doc.xpath('//message/source')]

    with open(output, 'w') as stream:
        stream.write('\n\n'.join(texts))

if __name__ == "__main__":
    main(*sys.argv[1:])
