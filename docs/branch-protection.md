# Branch Protection Recommendation

Recommended GitHub branch protection for `main`:

## Required settings

- Require a pull request before merging
- Require approvals: `1`
- Dismiss stale approvals when new commits are pushed
- Require status checks to pass before merging
- Require branches to be up to date before merging
- Require linear history
- Restrict who can push to matching branches: enabled for maintainers only

## Required checks

- `CI / ubuntu-latest / gcc`
- `CI / ubuntu-latest / clang`
- `CI / macos-latest / clang`
- `CI / windows-latest / msvc`
- `package / ubuntu-latest`
- `package / macos-latest`
- `package / windows-latest`

## Suggested merge policy

- Prefer squash merge for feature work
- Use merge commits only when preserving branch history matters

## Notes

- Keep release tags separate from branch protection rules.
- Keep the package jobs required so release artifacts stay consumable, not just buildable.
