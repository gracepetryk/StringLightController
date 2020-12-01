from flask import Flask, Response, request
from lightControl import LightControl

app = Flask(__name__)
lc = LightControl()


@app.route('/on')
def turnOn():
    lc.turn_on()
    return Response("On", status=200)


@app.route('/off')
def turnOff():
    lc.turn_off()
    return Response("Off", status=200)


@app.route("/setcolor")
def set_color():
    r = int(request.args.get("r"))
    g = int(request.args.get("g"))
    b = int(request.args.get("b"))
    lc.set_color(r, g, b)
    return Response("set color to {}, {}, {}".format(r, g, b), status=200)




