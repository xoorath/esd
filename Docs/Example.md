# Electrostatic Discharge

## Example

[Home](../README.md) / [Docs](./Readme.md) / *Example*

ESD supports including files, declaring variables and variable substitution. You can see them in action in the following example:

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

**ESD is simple by design. The rest is up to you.**
