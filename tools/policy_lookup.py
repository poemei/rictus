#!/usr/bin/env python3
"""Read-only, literal evidence lookup across a STN-LABZ full-text policy catalog.

[AI:GPT-6 | 2026-11-04 05:23:32 UTC]
"""
import argparse
import json
import sys


def lookup(catalog, phrase, limit=20):
    """Return cited lines matching a literal phrase; never decide applicability."""
    if not isinstance(catalog, dict) or not isinstance(catalog.get("documents"), list):
        raise ValueError("Invalid catalog: missing documents list")
    if not isinstance(phrase, str) or not phrase.strip():
        raise ValueError("A nonempty literal query is required")
    if not isinstance(limit, int) or not 1 <= limit <= 200:
        raise ValueError("Limit must be between 1 and 200")
    needle = phrase.casefold()
    matches = []
    for doc in catalog["documents"]:
        if not isinstance(doc, dict) or not isinstance(doc.get("text"), str):
            raise ValueError("Full-text catalog required; rerun inventory --include-text")
        for line_no, line in enumerate(doc["text"].splitlines(), start=1):
            if needle in line.casefold():
                matches.append({
                    "file": doc.get("file", ""),
                    "root_document_id": doc.get("root_document_id", ""),
                    "revision_id": doc.get("revision_id", ""),
                    "status_declared": doc.get("status_declared", ""),
                    "line": line_no,
                    "text": line,
                    "sha256_file_bytes": doc.get("sha256_file_bytes", ""),
                    "metadata_issues": doc.get("issues", []),
                })
    return {"query": phrase, "total_matches": len(matches),
            "matches": matches[:limit], "truncated": len(matches) > limit,
            "interpretation": "Literal evidence matches only. Not a determination of authority, applicability, approval, or permission."}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("catalog", help="JSON produced by policy_inventory.py --include-text")
    parser.add_argument("phrase", help="Case-insensitive literal phrase to find")
    parser.add_argument("--limit", type=int, default=20)
    args = parser.parse_args()
    try:
        with open(args.catalog, "r", encoding="utf-8") as stream:
            catalog = json.load(stream)
        result = lookup(catalog, args.phrase, args.limit)
    except (OSError, ValueError, json.JSONDecodeError) as error:
        parser.error(str(error))
    json.dump(result, sys.stdout, ensure_ascii=False, indent=2)
    sys.stdout.write("\n")


if __name__ == "__main__":
    main()
# [End AI:GPT-6]
