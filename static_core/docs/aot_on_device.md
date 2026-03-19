# AOT on Device

## Framework AOT

Frameworks include ABCs shared by many apps. These files are typically provided through `--boot-panda-files`.
There are two strategies to enable framework AOT on device.

1. Integrate into the device image so the files live under `/system/framework`

2. Compile later on device while it is idle so the generated `.an` files appear under
   `/data/service/el1/public/for-all-app/framework_ark_cache`

AOT files under `/data` are usually newer than those from `/system`, because device-side recompilation can use PGO and
store refreshed artifacts in the runtime cache. Before the first device-side compilation finishes, the runtime still
needs to fall back to the `.an` file next to the original `.abc` under `/system/framework`.

In practice, `--boot-panda-files` may contain several boot ABCs. Some of their matching `.an` files may be present in
the boot AOT cache, while others may only exist next to the ABC itself.

Therefore, runtime supports loading AOT files from both locations. `--enable-an` enables the lookup, and
`--boot-an-location` provides the preferred boot-cache directory. On device, `--boot-an-location` is normally set to
`/data/service/el1/public/for-all-app/framework_ark_cache`, because that cache is expected to contain the newest
artifacts. If no cached `.an` file is found there, runtime falls back to the directory of the corresponding `.abc`.

The current lookup order for boot panda files is:

```
bool FileManager::TryLoadAnFileForLocation(std::string_view abcPath)
{
    PandaString::size_type posStart = abcPath.find_last_of('/');
    PandaString::size_type posEnd = abcPath.find_last_of('.');
    if (posStart == std::string_view::npos || posEnd == std::string_view::npos) {
        return true;
    }
    LOG(DEBUG, PANDAFILE) << "current abc file path: " << abcPath;
    PandaString abcFilePrefix = PandaString(abcPath.substr(posStart, posEnd - posStart));
    
    // If set boot-an-location, load from this location first
    std::string_view anLocation = Runtime::GetOptions().GetBootAnLocation();
    if (!anLocation.empty()) {
        bool res = FileManager::TryLoadAnFileFromLocation(anLocation, abcFilePrefix, abcPath);
        if (res) {
            return true;
        }
    }

    // If load failed from boot-an-location, continue try load from location of abc
    anLocation = abcPath.substr(0, posStart);
    FileManager::TryLoadAnFileFromLocation(anLocation, abcFilePrefix, abcPath);
    return true;
}
```
