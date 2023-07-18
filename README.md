# Kemai, a Kimai Desktop Client


## Build Status

|Develop|Master|Translation|
|:--:|:--:|:--:|
|![Build status](https://github.com/AlexandrePTJ/kemai/actions/workflows/main.yml/badge.svg?branch=develop)|![Build status](https://github.com/AlexandrePTJ/kemai/actions/workflows/main.yml/badge.svg?branch=master)|[![Translation status](https://hosted.weblate.org/widgets/kemai/-/kemai/svg-badge.svg)](https://hosted.weblate.org/engage/kemai/)|

## How to install

Several methods to install _Kemai_ are available.


### From GitHub releases

Pre-build packages are available from [Releases](https://github.com/AlexandrePTJ/kemai/releases/latest). Just select the asset for your OS.

Binaries with `-NoUpdate` suffix will not automaticaly check for new update. Thoses binaries are intent to be use with OS specifics packages manager.


### From Homebrew (mac OS only)

A [Tap](https://github.com/AlexandrePTJ/homebrew-cask) repository is available to simplify _Kemai_ install through `brew`

```shell script
> brew tap alexandreptj/cask
> brew install kemai
```


### From source

_Kemai_ is Qt6 based application. It uses CMake to build. So regular cmake process will work.

```shell script
> cmake . -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH=<path to Qt installation>
> cmake --build build --config Release
```


## How to use

_Kemai_ connects to your _Kimai_ instance through its API. As credentials for API are not the same as login, here is how to create them:

![API password](https://github.com/AlexandrePTJ/kemai/blob/master/docs/api_password.gif)

Then, you can set this credentials to _Kemai_ settings :

![Kemai settings](https://github.com/AlexandrePTJ/kemai/blob/master/docs/kemai_settings.gif)


## How to help

Ideas, pull requests and translation are welcome.

For the later, [Weblate](https://hosted.weblate.org/engage/kemai/) is used. Here is the current status:

[![Translation status](https://hosted.weblate.org/widgets/kemai/-/kemai/multi-auto.svg)](https://hosted.weblate.org/engage/kemai/)


## Why Kemai

Because "Il n'y a que Maille qui m'aille".
