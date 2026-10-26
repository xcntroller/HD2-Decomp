"""Synthetic stdlib tests; no Ghidra, game files, or scratch directories needed."""

from copy import deepcopy
import unittest

from function_progress import (
    END, HASHES, ROOT, START, make_baseline, render, replace_block,
    unique_object, validate_baseline, validate_inventory, validate_ledger,
)


def function(entry, *, external=False, thunk=False):
    return {
        "entry": entry, "name": "synthetic", "namespace": "Global",
        "name_source": "DEFAULT", "tags": [], "external": external, "thunk": thunk,
        "body_bytes": 1 if external else 16,
        "body_ranges": [] if external else [[entry, f"{int(entry, 16) + 15:08x}"]],
        "classification": "external" if external else "internal_thunk" if thunk else "unclassified",
    }


def fixtures():
    inventories = {}
    for program, entry in (("client", "00401000"), ("engine", "10001000")):
        inventories[program] = {
            "schema_version": 1, "program": program, "sha256": HASHES[program],
            "program_name": program, "ghidra_version": "synthetic", "image_base": entry,
            "functions": [
                function(entry),
                function(f"{int(entry, 16) + 32:08x}", thunk=True),
                function("EXTERNAL:00000001", external=True),
            ],
        }
    ledger = {
        "schema_version": 1, "programs": dict(HASHES),
        "functions": [{
            "program": "client", "entry": "00401000", "status": "partial",
            "coverage": "Synthetic supported subset", "limitations": "Excluded behavior",
            "evidence": ["README.md"],
        }],
    }
    return inventories, ledger


class FunctionProgressTests(unittest.TestCase):
    def setUp(self):
        self.inventories, self.ledger = fixtures()
        self.functions, self.baseline = make_baseline(self.inventories)

    def row(self):
        return self.ledger["functions"][0]

    def check_ledger(self):
        return validate_ledger(self.ledger, self.functions, ROOT)

    def check_inventory(self):
        return validate_inventory(self.inventories["client"], "client")

    def test_census_counts_external_and_thunks_separately(self):
        counts = self.baseline["programs"]["client"]["counts"]
        self.assertEqual(counts, {
            "internal_non_thunk": 1, "internal_thunk": 1, "external": 1,
            "identified_total": 3, "unclassified_internal_non_thunk": 1,
        })

    def test_partial_earns_no_completion_and_renders_zero_bar(self):
        statuses = self.check_ledger()
        output = render(self.baseline, statuses)
        self.assertIn("[------------------------------] 0.00% complete", output)
        self.assertIn("**0 complete / 2 identified", output)
        self.assertIn("**1 unique functions partially reconstructed**", output)

    def test_valid_complete_requires_explicit_end_to_end_and_verification(self):
        self.row().update(status="complete", limitations="", end_to_end=True,
                          verification="Synthetic verification, not a real reconstruction claim")
        self.assertEqual(self.check_ledger()["client"]["complete"], 1)
        self.assertIn("50.00%", render(self.baseline, self.check_ledger()))

    def test_empty_ledger_is_allowed(self):
        self.ledger["functions"] = []
        self.assertEqual(sum(v["complete"] for v in self.check_ledger().values()), 0)

    def test_name_changes_do_not_reset_structural_baseline(self):
        self.inventories["client"]["functions"][0]["name"] = "renamed"
        _, revised = make_baseline(self.inventories)
        validate_baseline(self.baseline, revised)

    def test_inventory_order_is_irrelevant(self):
        self.inventories["client"]["functions"].reverse()
        _, revised = make_baseline(self.inventories)
        validate_baseline(self.baseline, revised)

    def test_function_discovery_requires_deliberate_baseline_change(self):
        self.inventories["client"]["functions"].append(function("00402000"))
        _, revised = make_baseline(self.inventories)
        with self.assertRaises(ValueError):
            validate_baseline(self.baseline, revised)

    def test_baseline_schema_type_is_not_boolean(self):
        revised = deepcopy(self.baseline)
        revised["schema_version"] = True
        with self.assertRaises(ValueError):
            validate_baseline(self.baseline, revised)

    def test_wrong_inventory_fingerprint_and_program(self):
        for field, value in (("sha256", "0" * 64), ("program", "expansion"),
                             ("schema_version", True)):
            with self.subTest(field=field):
                document = deepcopy(self.inventories["client"])
                document[field] = value
                with self.assertRaises(ValueError):
                    validate_inventory(document, "client")

    def test_duplicate_inventory_entry_is_rejected(self):
        self.inventories["client"]["functions"].append(function("00401000"))
        with self.assertRaises(ValueError):
            self.check_inventory()

    def test_empty_inventory_is_rejected(self):
        self.inventories["client"]["functions"] = []
        with self.assertRaises(ValueError):
            self.check_inventory()

    def test_bad_body_shapes_and_sizes_are_rejected(self):
        bad_values = [
            {"body_ranges": [["00401010", "00401000"]]},
            {"body_ranges": [["00401000", "0040100f"], ["0040100f", "00401010"]]},
            {"body_ranges": [["00401001", "00401010"]]},
            {"body_ranges": [["401000", "0040100f"]]},
            {"body_bytes": 17}, {"body_bytes": True}, {"body_bytes": -1},
            {"external": "false"}, {"classification": "game"},
        ]
        for changes in bad_values:
            with self.subTest(changes=changes):
                document = deepcopy(self.inventories["client"])
                document["functions"][0].update(changes)
                with self.assertRaises(ValueError):
                    validate_inventory(document, "client")

    def test_bad_external_address_and_body_are_rejected(self):
        for changes in ({"entry": "EXTERNAL:bogus"},
                        {"body_ranges": [["00401000", "00401001"]]}):
            with self.subTest(changes=changes):
                document = deepcopy(self.inventories["client"])
                document["functions"][2].update(changes)
                with self.assertRaises(ValueError):
                    validate_inventory(document, "client")

    def test_external_thunk_counts_once(self):
        self.inventories["client"]["functions"][2]["thunk"] = True
        _, summary = self.check_inventory()
        self.assertEqual(summary["counts"]["identified_total"], 3)
        self.assertEqual(summary["counts"]["internal_thunk"], 1)
        self.assertEqual(summary["counts"]["external"], 1)

    def test_duplicate_ledger_entry_rejects_double_fragment_credit(self):
        self.ledger["functions"].append(deepcopy(self.row()))
        with self.assertRaises(ValueError):
            self.check_ledger()

    def test_invalid_status_entry_program_and_unexpected_keys(self):
        for changes in ({"status": "understood"}, {"entry": "00401001"},
                        {"entry": "00401020"}, {"entry": "EXTERNAL:00000001"},
                        {"program": "expansion"}, {"entry": "00401ABC"},
                        {"complete": True}):
            with self.subTest(changes=changes):
                row = deepcopy(self.row())
                row.update(changes)
                ledger = deepcopy(self.ledger)
                ledger["functions"] = [row]
                with self.assertRaises(ValueError):
                    validate_ledger(ledger, self.functions, ROOT)

    def test_mismatched_ledger_fingerprint(self):
        self.ledger["programs"]["engine"] = HASHES["client"]
        with self.assertRaises(ValueError):
            self.check_ledger()

    def test_partial_must_have_limitations(self):
        self.row()["limitations"] = ""
        with self.assertRaises(ValueError):
            self.check_ledger()

    def test_partial_cannot_claim_end_to_end(self):
        self.row()["end_to_end"] = True
        with self.assertRaises(ValueError):
            self.check_ledger()

    def test_incomplete_claim_cannot_be_promoted(self):
        self.row()["status"] = "complete"
        with self.assertRaises(ValueError):
            self.check_ledger()

    def test_missing_unsafe_and_duplicate_evidence(self):
        for links in ([], ["missing-progress-evidence"], ["/etc/passwd"],
                      ["../README.md"], ["README.md", "README.md"]):
            with self.subTest(links=links):
                self.row()["evidence"] = links
                with self.assertRaises(ValueError):
                    self.check_ledger()

    def test_valid_optional_coverage_intervals(self):
        self.row()["intervals"] = [["00401000", "00401003"], ["00401008", "0040100f"]]
        self.assertEqual(self.check_ledger()["client"]["partial"], 1)

    def test_malformed_or_out_of_body_coverage_is_rejected(self):
        for value in ([], [["00401004", "00401003"]],
                      [["00401000", "0040100f"], ["0040100f", "00401010"]],
                      [["00401000", "00401010"]], [["00401000", 1]]):
            with self.subTest(value=value):
                self.row()["intervals"] = value
                with self.assertRaises(ValueError):
                    self.check_ledger()

    def test_complete_cannot_claim_only_some_intervals(self):
        self.row().update(status="complete", limitations="", end_to_end=True,
                          verification="Synthetic", intervals=[["00401000", "00401003"]])
        with self.assertRaises(ValueError):
            self.check_ledger()
        self.row()["intervals"] = [["00401000", "0040100f"]]
        self.assertEqual(self.check_ledger()["client"]["complete"], 1)

    def test_json_duplicate_keys_are_rejected(self):
        with self.assertRaises(ValueError):
            unique_object([("status", "partial"), ("status", "complete")])

    def test_render_is_deterministic_and_preserves_other_readme_text(self):
        block = render(self.baseline, self.check_ledger())
        original = f"Before\n{START}\nstale\n{END}\nAfter\n"
        expected = f"Before\n{block}\nAfter\n"
        self.assertEqual(replace_block(original, block), expected)
        self.assertEqual(replace_block(expected, block), expected)

    def test_missing_duplicate_reversed_markers_rejected(self):
        for readme in ("no markers", START + END + START, END + START):
            with self.subTest(readme=readme), self.assertRaises(ValueError):
                replace_block(readme, "replacement")


if __name__ == "__main__":
    unittest.main()
