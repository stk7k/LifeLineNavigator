
// TEMPORARY TEST CREDENTIALS
const testAccounts = {
    admin: {
        username: "admin",
        password: "123",
        redirect: "/admin.html"
    },

    patient: {
        username: "user",
        password: "123",
        redirect: "/patient.html"
    }
};

function handleLogin(formId, usernameId, passwordId, role) {
    const form = document.getElementById(formId);

    if (!form) return;

    form.addEventListener("submit", function (event) {
        event.preventDefault();

        const username = document.getElementById(usernameId).value.trim();
        const password = document.getElementById(passwordId).value;

        const account = testAccounts[role];

        if (username === account.username && password === account.password) {
            sessionStorage.setItem("loggedInRole", role);

            window.location.href = account.redirect;
        } 
        else {
            alert("Invalid username or password.");
        }
    });
}

handleLogin(
    "adminLoginForm",
    "adminUsername",
    "adminPassword",
    "admin"
);

handleLogin(
    "patientLoginForm",
    "patientUsername",
    "patientPassword",
    "patient"
);
