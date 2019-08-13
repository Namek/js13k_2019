// ultra simple auto-reload!
(() => {
  const watchClassTag = 'watch-for-auto-reload'
  const firstCheckDelay = 100
  const checksDelay = 500
  const additionalFilesToWatch = [
    'webgl.js',
    'triangles.wasm'
  ]
  const consoleLoggingEnabled = true


  function startCheckingFile(href) {
    const state = { lastModified: null }
    const isStyle = href.indexOf('.css') == href.length - 4

    // ask server about every 500ms for the file
    setInterval(() => {
      let xhr = new XMLHttpRequest()
      xhr.open("HEAD", href)
      xhr.send(null)

      xhr.onload = e => {
        const lastModified = xhr.getResponseHeader("last-modified")

        if (state.lastModified === null) {
          state.lastModified = lastModified

          if (consoleLoggingEnabled)
            console.log('watching file: ' + href + ', date: ' + lastModified)
        }
        else if (state.lastModified != lastModified) {
          state.lastModified = lastModified

          if (consoleLoggingEnabled)
            console.log('file changed: ' + href)

          if (isStyle) {
            replaceStylesheet(href)
          }
          else {
            location.reload()
          }
        }
      }
    }, checksDelay)
  }

  function replaceStylesheet(filename) {
    document.querySelectorAll('link').forEach(link => {
      if (link.href.indexOf(filename) < 0)
        return

      const queryString = '?reload=' + new Date().getTime()
      link.href = link.href.replace(/\?.*|$/, queryString)
    })
  }

  setTimeout(() => {
    document.querySelectorAll('.' + watchClassTag)
      .forEach(link => startCheckingFile(link.href))

    additionalFilesToWatch
      .forEach(startCheckingFile)

  }, firstCheckDelay)
})()