msgbox = ->
  dialog = new NKWinTaskDialog()
  dialog.show
    content: Array::slice.apply(arguments)

if process.scriptArgv.length == 0
  msgbox "No input files."
  process.exit 1

for file in process.scriptArgv
  bytecode = Duktape.dumpBytecode(Duktape.compile(fs.readFileSync(file).toString("utf-8"), "(app)", 0))
  fs.writeFileSync(file + "c", bytecode)
