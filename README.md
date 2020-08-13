# Kemai, a Kimai Desktop Client


## Build Status

|Develop|Master|
|:--:|:--:|
|![Build status](https://ci.appveyor.com/api/projects/status/github/AlexandrePTJ/Kemai?branch=develop&svg=true)|![Build status](https://ci.appveyor.com/api/projects/status/github/AlexandrePTJ/Kemai?branch=master&svg=true)|


## How to use

_Kemai_ connect to your _Kimai_ instance through its API.
As credentials for API are not the same as login, here is how to create them:

![API password](https://github.com/AlexandrePTJ/kemai/blob/master/docs/api_password.gif)

Then, you can set this credentials to _Kemai_ settings :

![Kemai settings](https://github.com/AlexandrePTJ/kemai/blob/master/docs/kemai_settings.gif)


## How to build

_Kemai_ is Qt5 based application and it uses CMake to build. So regular cmake process will work.

```shell script
> cmake . -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH=<path to Qt installation>
> cmake --build build --config Release
```

## Why Kemai

Because "Il n'y a que Maille qui m'aille".