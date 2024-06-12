from flask import Flask, render_template, request, session, redirect, url_for
from datetime import datetime

app = Flask(__name__)
app.secret_key = 'supersecretkey'  # Replace with a strong secret key

movement_history = []

@app.route("/", methods=["GET", "POST"])
def movement():
    global movement_history
    # Movement from Page Input
    if request.method == "POST":
        if request.form.get("reset"):
            movement_history = []
            return redirect(url_for("movement"))
        else:
            movement = request.form.get("movement")
            if movement is not None:
                movement = int(movement)  # Ensure movement is an integer
                timestamp = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
                movement_history.append({"movement": movement, "timestamp": timestamp})
                return redirect(url_for("movement"))

    # Movement from Arduino
    movement = request.args.get("movement")
    if movement is not None:
        movement = int(movement)
        timestamp = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
        movement_history.append({"movement": movement, "timestamp": timestamp})
        return redirect(url_for("movement"))

    return render_template("index.html", movement_history=movement_history)

if __name__ == "__main__":
    app.run(debug=True)