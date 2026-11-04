#!/usr/bin/env python3
"""Export a private, read-only line index for the Rictus evidence service.

[AI:GPT-6 | 2026-11-04 05:40:09 UTC]
"""
import argparse
import json
import re
import sys

IDENTIFIER = re.compile(r"^\d{8}\.\d+$")


def export_index(catalog):
    """Keep all revisions, including R0; do not select governing documents."""
    if not isinstance(catalog, dict) or not isinstance(catalog.get("documents"), list):
        raise ValueError("Full-text policy catalog required")
    rows = []
    for doc in catalog["documents"]:
        if not isinstance(doc, dict) or not isinstance(doc.get("text"), str):
            raise ValueError("Full-text policy catalog required")
        root = doc.get("root_document_id")
        if not isinstance(root, str) or not IDENTIFIER.fullmatch(root):
            raise ValueError("Invalid document root ID")
        for number, line in enumerate(doc["text"].splitlines(), start=1):
            clean = line.replace("\t", " ").replace("\r", " ")
            if len((root + "\t" + str(number) + "\t" + clean + "\n").encode("utf-8")) >= 4096:
                raise ValueError(f"Index line too long: {root}:{number}")
            rows.append(f"{root}\t{number}\t{clean}\n")
    return "".join(rows)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("catalog", help="Full-text policy catalog JSON")
    args = parser.parse_args()
    try:
        with open(args.catalog, encoding="utf-8") as stream:
            index = export_index(json.load(stream))
    except (OSError, ValueError, json.JSONDecodeError) as error:
        parser.error(str(error))
    sys.stdout.write(index)


if __name__ == "__main__":
    main()
# [End AI:GPT-6]
