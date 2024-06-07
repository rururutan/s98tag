# S98 Tag Tool

`s98tag` is a program that supports tag editing for s98 files.

## Usage

> s98tag [options] [input file]

**options**

        -a[tagname]=[value] ... Set tag value
        -i ... Show tags
        -u ... Convert to UTF-8 tags
        -m ... Convert to Shift JIS tags (Characters that cannot be converted to ShiftJIS are replaced with "?" character.)

Wildcards can be used for file names.

## Example

Register or replace tags

> s98tag -aTitle=Sample filename

Delete tag

> s98tag -aTitle= filename

Show tags

> s98tag -i filename

Convert to UTF-8 tags

> s98tag -u filename

Convert to Shit JIS tags

> s98tag -m filename

## Limitation

Files will be overwritten. Please back up your data before editing.

Only uncompressed S98 is supported.If the file is compressed with zip, please extract it beforehand.

## License

	MIT

