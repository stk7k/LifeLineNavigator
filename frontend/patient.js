
// Protect the Patient Dashboard
if (sessionStorage.getItem("loggedInRole") !== "patient") {
    window.location.replace("/patient-login.html");
}


// Patient navigation
const patientNavButtons = document.querySelectorAll("[data-patient-page]");
const patientPages = document.querySelectorAll(".page");

const patientPageNames = {
    overview: "Overview",
    profile: "My Profile",
    doctor: "My Doctor",
    beds: "Bed Information",
    billing: "Billing"
};

function showPatientPage(page) {

    patientPages.forEach(section => {
        section.classList.toggle(
            "active",
            section.id === `page-${page}`
        );
    });

    document.querySelectorAll(".nav-link[data-patient-page]").forEach(button => {
        button.classList.toggle(
            "active",
            button.dataset.patientPage === page
        );
    });

    document.getElementById("pageTitle").textContent =
        patientPageNames[page] || "Overview";

    document.getElementById("sidebar").classList.remove("open");
}


// Navigation click events
patientNavButtons.forEach(button => {

    button.addEventListener("click", () => {
        showPatientPage(button.dataset.patientPage);
    });

});


// Mobile sidebar
document.getElementById("mobileMenu").addEventListener("click", () => {
    document.getElementById("sidebar").classList.toggle("open");
});


// Today's date
document.getElementById("todayLabel").textContent =
    new Date().toLocaleDateString("en-IN", {
        day: "numeric",
        month: "short",
        year: "numeric"
    });


// Logout
document.getElementById("patientLogout").addEventListener("click", () => {

    sessionStorage.removeItem("loggedInRole");

    window.location.href = "/patient-login.html";

});


// Temporary demo patient data
const patient = {
    name: "Rahul Sharma",
    id: 1001,
    specialty: "General Medicine",
    admissionDate: "11 Nov 2026",
    status: "Admitted",
    doctor: "Dr. Amit Verma",
    doctorSpecialty: "General Medicine",
    ward: "General",
    bed: "02",
    bill: "₹1,300",
    payment: "Pending"
};


// Populate patient information
const setText = (id, value) => {
    const element = document.getElementById(id);
    if (element) element.textContent = value;
};

[
    ["patientName", patient.name],
    ["patientId", `#${patient.id}`],
    ["detailName", patient.name],
    ["detailId", `#${patient.id}`],
    ["detailSpecialty", patient.specialty],
    ["admissionDate", patient.admissionDate],
    ["admissionStatus", patient.status],
    ["doctorName", patient.doctor],
    ["doctorSpecialty", patient.doctorSpecialty],
    ["bedNumber", `${patient.ward} - ${patient.bed}`],
    ["billAmount", patient.bill],
    ["paymentStatus", `${patient.payment} payment`],

    ["profileName", patient.name],
    ["profileId", `#${patient.id}`],
    ["profileSpecialty", patient.specialty],
    ["profileAdmissionDate", patient.admissionDate],
    ["profileStatus", patient.status],

    ["profileDoctorName", patient.doctor],
    ["profileDoctorSpecialty", patient.doctorSpecialty],

    ["bedWard", patient.ward],
    ["bedDetailNumber", patient.bed],
    ["bedAdmissionStatus", patient.status],

    ["billingAmount", patient.bill],
    ["billingStatus", patient.payment]
].forEach(([id, value]) => setText(id, value));
