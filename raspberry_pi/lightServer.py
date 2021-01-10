from flask import Flask, Response, request
from lightControl import LightControl

app = Flask(__name__)
lc = LightControl()


@app.route('/on')
def turn_on():
    lc.turn_on()
    return Response("On", status=200)


@app.route('/off')
def turn_off():
    lc.turn_off()
    return Response("Off", status=200)


@app.route("/powerstate")
def get_power_state():
    return Response(str(lc.get_on_off() == b'\xFF'), status=200)


@app.route("/setcolor")
def set_color():
    r = int(request.args.get("r"))
    g = int(request.args.get("g"))
    b = int(request.args.get("b"))
    lc.set_color(r, g, b)
    return Response("set color to {}, {}, {}".format(r, g, b), status=200)


@app.route("/mode_solid")
def mode_solid():
    lc.set_mode(lc.MODE_SOLID_BYTE)
    return Response("Set mode to solid", status=200)


@app.route("/mode_fade")
def mode_fade():
    lc.set_mode(lc.MODE_FADE_BYTE)
    return Response("Set mode to fade", status=200)


@app.route("/mode_jump")
def mode_jump():
    lc.set_mode(lc.MODE_JUMP_BYTE)
    return Response("Set mode to jump", status=200)


@app.route("/mode_fade_async")
def mode_fade_async():
    lc.set_mode(lc.MODE_FADE_ASYNC_BYTE)
    return Response("Set mode to async fade", status=200)


@app.route("/mode_jump_async")
def mode_jump_async():
    lc.set_mode()
    return Response("Set mode to async jump", status=200)

