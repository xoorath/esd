# Variables

[Home](../README.md) / [Docs](./Readme.md) / *Variables*

## Using Variables

The purpose of variables in ESD are to replace named values like `{$site_bg}` with a bit of text you [only have to write once](https://en.wikipedia.org/wiki/Don%27t_repeat_yourself).

For example if your Vars.txt file contains the line `site_bg=#000` then all instances of `{$site_bg}` in your website will be replaced with `#000`.

## Valid Names

A valid name should start with an underscore `_`, a hyphen `-` or a letter `a-z`, `A-Z` which is followed by any numbers, hyphens, underscores, letters. It cannot start with a digit.

## Vars.txt 

A project can have global variables defined in a `Vars.txt` file at the project root. 

Here's an overview of it's features in the form of an example Vars.txt:

### Example:

`/Vars.txt`

```
# Feature 1: Comments
# This is an example comment.
# A comment is any line with '#' as the first character.

# Feature 2: Variables
# A variable is any line with at least one '=' character in it. 
# The key and value are on either side of the first '=' character encountered.

site_title=Example Site Title

# Feature 3: Multiline Variables
# You can have multi-line values by utilizing a backslash as the last character on the line.
# The resulting variable value will include a newline but will not include a backslash.

multiline=This variable is multiline because it ends with a backslash\
this text is also part of the variable named multiline

# Edge cases:
# Variable names are case sensitive.
# If you require a backslash at the end of a line you can escape it with two backslashes.

unique_varname=Variables names are case sensitive.
UNIQUE_VARNAME=This variable is not the same as the one above.

escaped=A variable can end with a backslash if needed by escaping it with two backslashes.\\
```

## Inline Variables

In your source files in `/privates/site/` and `/private/components/` you can declare variables inline.

Inline variables do not support comments or multiline values, but are handy for keeping variables alongside where they're used such as color pallets in a css file or post details blog.

### Example:

`/privates/site/index.html`
```
<html>
    {variable:site_title=Example Site With Inline Variables}
    <head>
        <title>{$site_title}</title>
    </head>
    <body>
        <h1>{$site_title}</h1>
    </body>
</html>
```

## Conflicts and scope

It can be helpful to understand how variable scope works so you can develop with confidence. Luckily it's dead simple in ESD.

1. When a variable is declared multiple times the latest (furthest down) instance takes priority. You will get a warning when this happens.
2. All inline variables take priority over global variables from Vars.txt.
3. Variables are processed all at once, not as a file is rendered.

Let's take a closer look at the third point:

```
{variable:X=1}

<p>The value of x is {$X}.</p>

{variable:X=2}

<p>The value of x is {$X}.</p>
```

The above example will produce a warning about X being overwritten and the following page:

```
<p>The value of x is 2.</p>
<p>The value of x is 2.</p>
```

### Variables defined in components:

Let's look at how scope is handled when you declare variables in components. In esd all includes are processed first **all at once** and then all variables are processed in a second pass. Consider the following example:

`Private/Components/foo.html`
```
{variable:X=42}
```
`Private/Site/index.html`
```
{include:foo.html}
<h1>{$X}<h1>
```

The above example renders  `<h1>42</h1>` into the output `Public/index.html`. 

### Variables used in components:

Let's flip the above example and see further proof that includes are processed entirely before variables:


`Private/Components/foo.html`
```
<h1>{$X}<h1>
```
`Private/Site/index.html`
```
{include:foo.html}

{variable:X=42}
```

In the above example we still get `<h1>42</h1>` printed even though the component used a variable that may or may not exist (luckily index.html declares it). On top of that the variable was declared after foo was included! This example really illustrates the order the site is rendered in: includes then variables.