source common.sh

# Tests miscellaneous commands.

# Do all commands have help?
#nix-env --help | grep -q install
#nix-store --help | grep -q realise
#nix-instantiate --help | grep -q eval
#nix-hash --help | grep -q base32

# Can we ask for the version number?
nix-env --version | grep "$version"

# Usage errors.
nix-env --foo 2>&1 | grep "no operation"
nix-env -q --foo 2>&1 | grep "unknown flag"

# Eval Errors.
eval_res=$(nix-instantiate --eval -E 'let a = {} // a; in a.foo' 2>&1 || true)
echo $eval_res | grep "(string) (1:15)"
echo $eval_res | grep "infinite recursion encountered"
