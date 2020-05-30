# GOG Galaxy SDK extension for Defold

Extension that implements achievements using GOG Galaxy SDK

## Installation

**Some assembly required**

Due to licensing concerns, I did not bundle GOG Galaxy SDK with this extension.
You will have to copy the `gog_galaxy` directory into your own project, then
copy the Galaxy headers to `gog_galaxy/include/galaxy` and the libraries to `lib` 
and `res` according to the following schema:

![Library paths](./libraries.png)

Additionally, since Defold doesn't fully support linking against shared libraries,
once you add this extension, your game will bundle correctly, but won't run from the
editor anymore (unless you use some [hacky tricks](./run_osx.sh)).

My recommendation is to disable the extension for regular development (by renaming its
`ext.manifest` to something like `ext.manifest_disabled`) and only enable it (manually
or through your build script, if you have one) when doing GOG bundles.

## Usage

I don't have time to write documentation, but I included a 
[wrapper module](./gog_galaxy/gog.lua) that makes using it easier and 
handles errors gracefully. You can still use the module with the extension
disabled and it will just ignore all the calls to it. It's also safe to set
achievements before GOG logged in and initialised, as the wrapper will queue the calls.

Add `gog.client_id` and `gog.client_secret` to your `game.project` to initialise the 
module, then check out the usage [example](./example/main.script).