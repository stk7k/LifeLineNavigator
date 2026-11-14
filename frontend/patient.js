
// Protect the Patient Dashboard
if (sessionStorage.getItem("loggedInRole") !== "patient") {
    window.location.replace("/patient-login.html");
}

// Get logged-in patient ID
const patientId = Number(
    sessionStorage.getItem("loggedInPatientId")
);

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

// Navigation events
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
    sessionStorage.removeItem("loggedInPatientId");

    window.location.href = "/patient-login.html";
});

// Helper to update HTML elements
function setText(id, value) {
    const element = document.getElementById(id);

    if (element) {
        element.textContent = value;
    }
}

// Format paise into rupees
function formatMoney(paise) {
    return "₹" + (paise / 100).toLocaleString("en-IN", {
        minimumFractionDigits: 2,
        maximumFractionDigits: 2
    });
}

// Fetch patient information
// Fetch patient information
async function loadPatientDashboard() {
    try {
        if (!Number.isInteger(patientId) || patientId <= 0) {
            throw new Error("Invalid patient login. Please log in again.");
        }

        // Fetch this specific patient directly from Supabase
        const patientRes = await fetch(`/api/patients/${patientId}`);

        if (!patientRes.ok) {
            throw new Error("Could not retrieve your patient record.");
        }

        const patientData = await patientRes.json();

        if (!patientData.success || !patientData.patient) {
            throw new Error(
                patientData.message || "Patient record not found."
            );
        }

        const patient = patientData.patient;

        // Doctor information
        const doctorName = patient.doctorName || "Not assigned";
        const doctorSpecialty = patient.doctorSpecialty || "Not assigned";

        // Bed information
        const ward = patient.bedWard || "Not assigned";
        const bedNumber = patient.bedId || "Not assigned";

        // Admission date
        const admissionDate = patient.admissionDate
            ? new Date(patient.admissionDate).toLocaleDateString("en-IN", {
                day: "numeric",
                month: "short",
                year: "numeric"
            })
            : "Not available";

        // Fetch billing information
        let bill = null;

        const billRes = await fetch(`/api/bills/${patientId}`);

        if (billRes.ok) {
            const billData = await billRes.json();

            if (billData.success) {
                bill = billData;
            }
        }

        // Patient details
        setText("patientName", patient.name);
        setText("patientId", `#${patient.id}`);

        setText("detailName", patient.name);
        setText("detailId", `#${patient.id}`);
        setText("detailSpecialty", patient.specialty);

        setText("admissionStatus", patient.status);
        setText("profileStatus", patient.status);
        setText("bedAdmissionStatus", patient.status);

        setText("admissionDate", admissionDate);
        setText("profileAdmissionDate", admissionDate);

        // Doctor details
        setText("doctorName", doctorName);
        setText("doctorSpecialty", doctorSpecialty);

        setText("profileDoctorName", doctorName);
        setText("profileDoctorSpecialty", doctorSpecialty);

        // Bed details
        setText(
            "bedNumber",
            patient.bedId
                ? `${ward} - ${bedNumber}`
                : "Not assigned"
        );

        setText("bedWard", ward);
        setText("bedDetailNumber", bedNumber);

        // Billing details
        if (bill) {
            const total = formatMoney(bill.totalPaise);
            const billStatus = bill.closed ? "Finalized" : "Open";

            setText("billAmount", total);
            setText("billingAmount", total);

            setText("paymentStatus", `${billStatus} bill`);
            setText("billingStatus", billStatus);
        } else {
            setText("billAmount", "No bill yet");
            setText("billingAmount", "No bill yet");

            setText("paymentStatus", "No bill available");
            setText("billingStatus", "No bill available");
        }

    } catch (error) {
        console.error("Patient dashboard error:", error);

        setText("patientName", "Unable to load patient");
        setText("patientId", "—");
    }
}

// Load dashboard data
//loadPatientDashboard();

// Load immediately
loadPatientDashboard();

// Refresh patient information every 5 seconds
setInterval(loadPatientDashboard, 5000);