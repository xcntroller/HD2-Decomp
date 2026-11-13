# GitHub Pages deployment

Expected URL: **https://xcntroller.github.io/HD2-Decomp/**

The workflow is [`pages.yml`](../.github/workflows/pages.yml), named **Deploy
project website**. It runs on every push to `master`, with no path filter, so
ledger, plan, research and source changes can refresh the site. Manual dispatch
is supported. Both jobs reject other branches and forks. No deployment has been
performed as part of preparing this workflow.

## One-time repository settings

An administrator of `xcntroller/HD2-Decomp` should:

1. Open **Settings → Actions → General**. Enable Actions and permit the official
   `actions/*` actions used in the workflow if an action allowlist is configured.
   The workflow pins all actions to reviewed full commit SHAs. The repository's
   default workflow-token setting can remain read-only; job permissions explicitly
   grant the required Pages access. No personal token or new secret is required.
2. Open **Settings → Pages → Build and deployment → Source** and choose
   **GitHub Actions**, rather than branch-based publishing. Do not add a custom
   domain for the expected `github.io/HD2-Decomp/` URL. The workflow deliberately
   sets `configure-pages`'s `enablement: false`; it does not administer this setting.
3. Open **Settings → Environments → github-pages** (create it if absent). Set
   **Deployment branches and tags** to selected branches/tags and allow the branch
   **`master`**. Existing protection rules must allow this branch. Required reviewers
   are optional: enabling them pauses each publication until review; leave them
   unset for automatic deployment after master pushes.

These are manual instructions, not settings changed by the tooling.

## Activate deployment after approval

1. Review and approve the changes on `tooling/progress-analyzer`.
2. Integrate the tooling/website/workflow changes into `master` through the
   project's approved process, preserving independent reconstruction work.
3. Enable the settings above before the approved master push. **That push starts
   the first automatic deployment.** The workflow does not need a `gh-pages` branch.
4. Open **Actions → Deploy project website** and inspect the run for that master
   commit. `build` must pass before `deploy` can run. If the environment has a
   required reviewer, approve the environment deployment there.
5. Open the URL from the deployment job/environment, expected to be
   https://xcntroller.github.io/HD2-Decomp/. The snapshot SHA in the site and
   `/HD2-Decomp/data/progress.json` identifies the built revision.

Future pushes to `master` repeat the same pipeline without manual data generation.
Nothing in the local build pushes, merges, dispatches a workflow or deploys.

## Manual deployment

After the workflow exists on the default `master` branch:

1. Open **Actions → Deploy project website → Run workflow**.
2. Choose branch **`master`**, then **Run workflow**.
3. Watch the new `build` and `deploy` jobs and approve the environment only if its
   existing rules require review. Selecting a different branch skips both jobs.

An already authenticated GitHub CLI can also dispatch it; do not create a PAT
solely for this workflow:

```sh
gh workflow run pages.yml --repo xcntroller/HD2-Decomp --ref master
gh run list --repo xcntroller/HD2-Decomp --workflow pages.yml --branch master
# Substitute the resulting run ID:
gh run watch RUN_ID --repo xcntroller/HD2-Decomp
```

These commands publish when run against the enabled repository. They are supplied
as instructions and have not been executed by the implementation session.

## Local build and preview

Use Node 24 LTS, Python 3.10+ (CI selects 3.13), npm and Git. No Ghidra, Java,
commercial client/engine binaries, extracted assets or ignored inventories are
needed. From `website/`:

```sh
npm ci
npm run build
npm run preview
```

Open **http://127.0.0.1:4173/HD2-Decomp/**. Development uses `npm run dev` at
**http://127.0.0.1:5173/HD2-Decomp/**. Vite's `base` is `/HD2-Decomp/`, and routes
use hashes, for example `/HD2-Decomp/#/functions?address=00416e10`. Deep links and
reloads require no server-side rewrite or 404 fallback.

To inspect the data checks separately:

```sh
npm run data
npm run validate:data
python3 -B scripts/check_public.py dist
```

The last command checks an already built artifact; rebuild after changing source
metadata or the source/public revision. By default, local source links use the
branch's upstream commit and omit links for unpublished blobs. To select a known
public revision explicitly, set `HD2_PUBLIC_REF` to that commit/ref when building.
The builder is responsible for choosing an actually public ref for publication.
CI sets it to `github.sha` and checks that both snapshot and public links identify
that exact revision. An invalid explicit ref is a build error.

## Pipeline and validation boundary

The `build` job checks out the exact event revision with full history, without
LFS, submodules or persisted checkout credentials. It installs Node/Python and
runs `npm ci` from the lockfile. Only npm downloads are cached, using the lockfile
key. Snapshots and build output are never restored from a cache. The existing
binary-independent tracker tests run before the website build.

`npm run build` runs these ordered gates:

1. **Generate:** the existing analyzer reads checked-in progress metadata. The
   website adapter emits the allowlisted public JSON and Git activity metadata.
2. **Validate before Vite:** reject duplicate JSON keys, non-finite values,
   unsupported/missing fields, wrong types, stale input/snapshot revisions, unsafe
   links/private paths and changes from the current safe projection. Independent
   checks compare ledger counts, original function statuses, milestone states,
   node identities/verification levels and relationship direction/evidence strength
   with the analyzer. Unknown states stay unknown; inferred edges stay inferred.
3. **Compile:** run TypeScript and Vite with the Pages base path and source maps off.
4. **Audit after Vite:** require the current validated snapshot again. The output
   may contain only the built index, exact Vite-manifest assets (JS/CSS and the two
   licensed fonts), font licenses and public JSON. Undeclared files/directories,
   symlinks/hard links, invalid font headers, commercial file types, raw exports,
   logs and private machine paths fail publication. Vite's hidden manifest is for
   auditing and is excluded by the official uploader.

The validator does not execute game code or independently recertify historical
verification reports. Documented unknowns and the existing stale-cache notices
are valid data when preserved exactly and displayed honestly. They are not
silently converted to completion or an externally blocked state. A malformed
source ledger or unsupported analyzer schema still fails the pipeline.

After all gates succeed, `upload-pages-artifact` packages **only `website/dist/`**.
Artifacts expire after one day. `deploy-pages` runs in a separate dependent job
with `pages: write`, `id-token: write` and the `github-pages` environment. The
build job has only `contents: read` and `pages: read`. GitHub supplies the standard
`GITHUB_TOKEN` and OIDC identity; no database, PAT or custom secret is involved.
A shared `github-pages` concurrency group leaves the active run intact and retains
the newest pending run. GitHub-hosted deployment has not been tested locally;
local checks cover the workflow syntax and equivalent build/artifact gates.

## Local verification

The deployment change was checked with:

- `actionlint` 1.7.12 and workflow structure assertions for triggers, permissions,
  full action SHA pins, artifact path and the dependent deployment environment.
- The 27 existing tracker tests, all passing.
- A clean Git checkout with only tracked source and the deployment changes:
  offline locked dependency installation, snapshot generation/validation,
  TypeScript, Vite and the 14-file artifact audit all passed. No ignored research
  exports, Ghidra installation or commercial game files were copied or required.
- 56 temporary positive/negative deployment-gate checks, including corrupted JSON,
  promoted progress/verification, changed relationship provenance, unsafe links,
  private paths, extra files, symlinks, hard links and invalid font headers.
  Temporary audit sources were removed after verification.
- Byte-identical public snapshots across the working and clean checkouts at the
  same revision and public ref.
- Chromium against the built site: all five routes, prefixed snapshot/assets,
  lazy graph loading, function-address deep-link reload and mobile width passed,
  with no page errors, HTTP failures or requests outside `/HD2-Decomp/`.

This verifies the local pipeline; GitHub repository configuration, OIDC and a
live Pages deployment still require the approved first workflow run.

## Troubleshooting

The 2026-11-13 repair addresses `Invalid checkpoint verification` from the
analyzer when a receipt uses a list of report strings instead of keyed stages.
Both forms are supported; prose lists remain verbatim and unclassified in the
analyzer, with the original receipt linked from the website. Malformed list
elements still fail validation. Existing stage reports and all publication gates
remain in effect.

| Symptom | Check / action |
| --- | --- |
| No **Run workflow** button | Ensure `pages.yml` with `workflow_dispatch` is on the default `master` branch and your account can run workflows. |
| Both jobs skipped | Select `master`; the workflow intentionally rejects tooling branches and forks. |
| Configure Pages reports missing site / 404 | Set **Settings → Pages → Source → GitHub Actions**. Do not enable automatic repository administration or add a PAT. |
| Deployment waits for review / branch rejected | Check the `github-pages` environment's required reviewers and allowed branch rule for `master`. |
| Permission/OIDC failure | Check Actions organization policy and the deploy job's `pages: write` / `id-token: write`; keep the environment name unchanged. |
| Snapshot schema/count/provenance failure | Run `npm run data` then `npm run validate:data`; inspect the failing checked-in source contract. Never edit the generated JSON to force a success or suppress a required gate. |
| Invalid `HD2_PUBLIC_REF` / CI SHA mismatch | Use the exact checked-out event revision; in CI the workflow supplies `github.sha`. Local explicit refs must resolve in Git. |
| Artifact audit finds an unexpected file | Remove the source of that file from `public/` or the build. Do not copy repository folders or ignored research into `dist/`. New legitimate assets require deliberate review and an appropriate manifest/type allowance. |
| Artifact missing / expired when rerunning deploy | Dispatch a fresh run on `master` to regenerate, validate and upload a new artifact. Do not reuse a one-day-old artifact. |
| Blank page or asset 404s | Use the case-sensitive `/HD2-Decomp/` URL and verify that the uploaded directory was `website/dist`, not its parent. Use hash routes; root `/` is not this project's configured base. |
| Website appears behind source | Compare the site's snapshot SHA with the successful workflow commit, inspect build/deploy status, then reload. Only committed master inputs are published; another worktree's uncommitted work is not a source. |

References: [GitHub custom Pages workflows](https://docs.github.com/en/pages/getting-started-with-github-pages/using-custom-workflows-with-github-pages),
[Vite Pages base paths](https://vite.dev/guide/static-deploy.html#github-pages),
[official Pages uploader](https://github.com/actions/upload-pages-artifact),
[official Pages deployment action](https://github.com/actions/deploy-pages).
