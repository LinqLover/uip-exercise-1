#!/usr/bin/python3

# TODO: Currently breaks qt build process because the XML header is not 
# written back correctly. In particular, the case of the root tag matters!

import sys

from lxml import html


def main(input, output):

    with open(input, 'r') as stream:
        text = stream.read()
    with open(output, 'r') as stream:
        doc = html.fromstring(stream.read().encode())

    texts = text.split('\n\n')
    elements = [elem for elem in doc.xpath('//message/translation')]
    import pdb; pdb.set_trace()
    assert(len(texts) == len(elements))
    for text, element in zip(texts, elements):
        element.text = text
        try:
            if element.attrib['type'] == "unfinished":
                del element.attrib['unfinished']
            # TODO: Does not work
        except KeyError:
            pass

    with open(output, 'w') as stream:
        stream.write(html.tostring(doc).decode())

if __name__ == "__main__":
    main(*sys.argv[1:])
