#!/usr/bin/env python3
"""Regression tests for the private policy evidence index exporter.

[AI:GPT-6 | 2026-11-04 05:42:59 UTC]
"""
import unittest

from policy_runtime_index import export_index


class PolicyRuntimeIndexTests(unittest.TestCase):
    def test_all_versions_retain_identity_and_digest(self):
        catalog = {"documents": [
            {"root_document_id": "20260729.0", "revision_id": "NONE",
             "sha256_file_bytes": "a" * 64, "text": "Operational authority.\n"},
            {"root_document_id": "20260729.0", "revision_id": "20260729.0.R1",
             "sha256_file_bytes": "b" * 64, "text": "Revised authority.\n"},
        ]}
        rows = export_index(catalog).splitlines()
        self.assertEqual(len(rows), 2)
        self.assertEqual(rows[0].split("\t")[:3],
                         ["20260729.0", "1", "a" * 64])
        self.assertEqual(rows[1].split("\t")[:3],
                         ["20260729.0.R1", "1", "b" * 64])

    def test_text_tabs_are_normalized(self):
        catalog = {"documents": [{
            "root_document_id": "20260729.0", "revision_id": "NONE",
            "sha256_file_bytes": "a" * 64, "text": "Digit\tcontrol\n",
        }]}
        self.assertEqual(export_index(catalog).splitlines()[0].split("\t")[-1],
                         "Digit control")

    def test_missing_full_text_and_identity_are_rejected(self):
        with self.assertRaises(ValueError):
            export_index({"documents": [{"root_document_id": "20260729.0"}]})
        with self.assertRaises(ValueError):
            export_index({"documents": [
                {"root_document_id": "bad", "text": "x",
                 "sha256_file_bytes": "a" * 64}
            ]})


if __name__ == "__main__":
    unittest.main()
# [End AI:GPT-6]
