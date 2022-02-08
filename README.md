[![Build and Package](https://github.com/xoorath/esd/actions/workflows/cmake.yml/badge.svg)](https://github.com/xoorath/esd/actions/workflows/cmake.yml)

# esd

The esd tool is named after electrostatic discharge and is here to take the pain out of making static websites.

## Download

Download esd below or [build it yourself](Docs/Build.md)

[Linux x64](Todo/esd.zip) | [Windows x64](Todo/esd.zip) | [Windows x86](Todo/esd.zip) | [MacOS](Todo/esd.zip)

## Features

**There are only 3 features.**

Your website defined in `/Private/Site/*` is transformed with these three features and output into `/Public/*`.

* Files may be included from `/Private/Components/*` with `{include:file_name}`
* Variables can be declared in a global `./Vars.txt` or in any source file with `{variable:variable_name=example}`.
* Variables can be substituted with `{$variable_name}`.


## Example

The esd tool supports including files, declaring variables and variable substitution. You can see them in action in the following example:

`./Private/Sites/index.html`
```html
<html>
    {$site_title}
    <head>{include:head.html}</head>
    <body>
        <h1>{$site_title}</h1>
        <p>It's that {$difficulty}.</p>
    </body>
</html>
```
`./Private/Components/head.html`
```html
<title>{$site_title}</title>
```
`./Vars.txt`
```
site_title=Electrostatic Discharge
difficulty=easy
```

If you were to run `esd` in a directory with these files you would get the following output:

`Public/index.html`
```html
<html>
    <head><title>Electrostatic Discharge</title></head>
    <body>
        <h1>Electrostatic Discharge</h1>
        <p>It's that easy.</p>
    </body>
</html>
```

The esd tool is simple by design. The rest is up to you.


## Command line arguments

* The **`-v`** switch will enable verbose mode: outputting more debug information.

## Platforms

esd runs on Linux, Windows, and Macos.

## Docs

See the [docs](/Docs/Readme.md) page to learn more about building and using esd.

## License

See the [LICENSE](./LICENSE) file.