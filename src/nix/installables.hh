#pragma once

#include "util.hh"
#include "path.hh"
#include "eval.hh"
#include "flake/flake.hh"

#include <optional>

namespace nix {

struct DrvInfo;
struct SourceExprCommand;

namespace eval_cache { class EvalCache; class AttrCursor; }

struct Buildable
{
    std::optional<StorePath> drvPath;
    std::map<std::string, StorePath> outputs;
};

typedef std::vector<Buildable> Buildables;

struct App
{
    PathSet context;
    Path program;
    // FIXME: add args, sandbox settings, metadata, ...

    App(EvalState & state, Value & vApp);
};

struct Installable
{
    virtual ~Installable() { }

    virtual std::string what() = 0;

    virtual Buildables toBuildables() = 0;

    Buildable toBuildable();

    App toApp(EvalState & state);

    virtual std::pair<Value *, Pos> toValue(EvalState & state)
    {
        throw Error("argument '%s' cannot be evaluated", what());
    }

    /* Return a value only if this installable is a store path or a
       symlink to it. */
    virtual std::optional<StorePath> getStorePath()
    {
        return {};
    }

    virtual std::vector<std::pair<std::shared_ptr<eval_cache::AttrCursor>, std::string>>
    getCursor(EvalState & state, bool useEvalCache);
};

struct InstallableValue : Installable
{
    ref<EvalState> state;

    InstallableValue(ref<EvalState> state) : state(state) {}

    struct DerivationInfo
    {
        StorePath drvPath;
        StorePath outPath;
        std::string outputName;
    };

    virtual std::vector<DerivationInfo> toDerivations() = 0;

    Buildables toBuildables() override;
};

struct InstallableFlake : InstallableValue
{
    FlakeRef flakeRef;
    Strings attrPaths;
    Strings prefixes;
    const flake::LockFlags & lockFlags;

    InstallableFlake(ref<EvalState> state, FlakeRef && flakeRef,
        Strings && attrPaths, Strings && prefixes, const flake::LockFlags & lockFlags)
        : InstallableValue(state), flakeRef(flakeRef), attrPaths(attrPaths),
          prefixes(prefixes), lockFlags(lockFlags)
    { }

    std::string what() override { return flakeRef.to_string() + "#" + *attrPaths.begin(); }

    std::vector<std::string> getActualAttrPaths();

    Value * getFlakeOutputs(EvalState & state, const flake::LockedFlake & lockedFlake);

    std::tuple<std::string, FlakeRef, DerivationInfo> toDerivation();

    std::vector<DerivationInfo> toDerivations() override;

    std::pair<Value *, Pos> toValue(EvalState & state) override;

    std::vector<std::pair<std::shared_ptr<eval_cache::AttrCursor>, std::string>>
    getCursor(EvalState & state, bool useEvalCache) override;
};

ref<eval_cache::EvalCache> openEvalCache(
    EvalState & state,
    std::shared_ptr<flake::LockedFlake> lockedFlake,
    bool useEvalCache);

}
