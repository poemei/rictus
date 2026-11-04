#ifndef RICTUS_POLICY_EVIDENCE_H
#define RICTUS_POLICY_EVIDENCE_H

/* [AI:GPT-6 | 2026-11-04 05:39:50 UTC] */
#define RICTUS_POLICY_EVIDENCE_SERVICE "rictus.policy.evidence"
#define RICTUS_POLICY_EVIDENCE_PATH "/opt/rictus/modules/compliance/policy_index.tsv"

typedef struct {
    char operation[128];
    char target[64];
} rictus_policy_evidence_request_t;

typedef struct {
    int index_available;
    unsigned operation_matches;
    unsigned target_matches;
    char operation_document[64];
    char operation_sha256[65];
    unsigned operation_line;
    char target_document[64];
    char target_sha256[65];
    unsigned target_line;
} rictus_policy_evidence_response_t;
/* [End AI:GPT-6] */

#endif
