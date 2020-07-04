# LHP - LHP Hypertext Preprocessor
A Cloudflare Worker that lets you embed Lua code on a web page as if it were PHP.

**Demo:** Try out https://jse.li/lua and visit https://jse.li/lua?noexecute=1 to see the page's source code.

Instantly turn static sites dynamic in the *worst way possible!* Write HTML like this, and wrap any Lua code you want in `<?lua ... ?>` to be executed by the worker. A special function named `echo` is provided, which writes strings to the page.

```html
<!DOCTYPE html>
<html>
<head>
  <title>LHP demo</title>
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
</head>
<body>
  <p>LHP (LHP Hypertext Preprocessor)</p>
  <?lua
    function fib(n, a, b)
        if n == 0 then
            return a
        end
        if n == 1 then
            return b
        end
        return fib(n - 1, b, a + b)
    end

    function nth_fibonacci(n)
        return fib(n, 0, 1)
    end
  ?>

  <p>Running <?lua echo(_VERSION) ?></p>
  <p>Current time: <?lua echo(os.date()) ?></p>
  <p>The first 20 fibonacci numbers are:</p>
  <ul>
    <?lua
      for i = 1, 20 do
          echo("<li>" .. nth_fibonacci(i) .. "</li>")
      end
    ?>
  </ul>
</body>
</html>
```

[`index.js`](index.js) is the content of the Workers script.  
[`main.c`](src/main.c) is the c source code that calls into the lua library.  
[`build.js`](build.js) holds the command we use to call emscripten.  
[`webpack.config.js`](webpack.config.js) holds the webpack config we use to bundle the emscripten output together with the script.

This template requires [Docker](https://docs.docker.com/install/) for providing the emscripten build environment. While we believe this provides the best developer experience, if you wish to not use Docker you can delete the check for docker and the docker parts of the build command in `build.js`. Note this means you must have emscripten installed on your machine.

## Wrangler

This template requires version >=1.6.0 of [Wrangler](https://github.com/cloudflare/wrangler)

```console
$ wrangler generate myapp https://github.com/cloudflare/worker-emscripten-template
🔧   Creating project called `myapp`...
✨   Done! New project created /path/to/myapp
```

To demo you can use [`wrangler dev`](https://developers.cloudflare.com/workers/tooling/wrangler/commands/#dev-(alpha))

```console
$ wrangler dev
👂 Listening on http://localhost:8787
```

```console
$ curl http://localhost:8787/600*400.jpg?width=100
```

Shoutout to [Surma](https://twitter.com/dassurma) for his [webpack-emscripten-wasm](https://gist.github.com/surma/b2705b6cca29357ebea1c9e6e15684cc) gist that was instrumental in getting this working!
