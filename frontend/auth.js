// TEMPORARY ADMIN TEST CREDENTIALS
const adminAccount = {
    username: "admin",
    password: "123",
    redirect: "/admin.html"
};


// ADMIN LOGIN
function handleAdminLogin() {
    const form = document.getElementById("adminLoginForm");

    if (!form) return;

    form.addEventListener("submit", function (event) {
        event.preventDefault();

        const username = document.getElementById("adminUsername").value.trim();
        const password = document.getElementById("adminPassword").value;

        if (
            username === adminAccount.username &&
            password === adminAccount.password
        ) {
            sessionStorage.setItem("loggedInRole", "admin");
            sessionStorage.removeItem("loggedInPatientId");
            sessionStorage.removeItem("loggedInPatientName");

            window.location.href = adminAccount.redirect;
        } else {
            alert("Invalid username or password.");
        }
    });
}


// PATIENT LOGIN
function handlePatientLogin() {
    const form = document.getElementById("patientLoginForm");

    if (!form) return;

    form.addEventListener("submit", async function (event) {
        event.preventDefault();

        const username = document.getElementById("patientUsername").value.trim();
        const password = document.getElementById("patientPassword").value;

        const button = form.querySelector("button[type='submit']");

        if (button) button.disabled = true;

        try {
            const response = await fetch("/api/auth/login", {
                method: "POST",
                headers: {
                    "Content-Type": "application/json"
                },
                body: JSON.stringify({
                    username,
                    password
                })
            });

            const result = await response.json();

            if (!response.ok || !result.success) {
                throw new Error(result.message || "Invalid username or password.");
            }

            // Save the authenticated patient's details
            sessionStorage.setItem("loggedInRole", "patient");

            sessionStorage.setItem(
                "loggedInPatientId",
                result.patientId
            );

            sessionStorage.setItem(
                "loggedInPatientName",
                result.name
            );

            window.location.href = "/patient.html";

        } catch (error) {
            alert(error.message);
        } finally {
            if (button) button.disabled = false;
        }
    });
}


// INITIALISE LOGIN FOR BOTH PAGES
handleAdminLogin();
handlePatientLogin();