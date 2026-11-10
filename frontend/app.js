const $ = (selector, root = document) => root.querySelector(selector);
const $$ = (selector, root = document) => [...root.querySelectorAll(selector)];
const state = { patients: [], doctors: [], beds: [], currentBill: null };
const pageNames = { overview: 'Overview', patients: 'Patients', queue: 'Admission queue', doctors: 'Doctors', beds: 'Bed management', billing: 'Billing' };

async function api(path, options = {}) {
  const response = await fetch(`/api${path}`, {
    ...options,
    headers: { 'Content-Type': 'application/json', ...(options.headers || {}) }
  });
  let data;
  try { data = await response.json(); } catch { throw new Error('The server returned an unreadable response.'); }
  if (!response.ok || data.success === false) throw new Error(data.message || `Request failed (${response.status}).`);
  return data;
}
function money(paise) { return new Intl.NumberFormat('en-IN', { style: 'currency', currency: 'INR', maximumFractionDigits: 2 }).format((paise || 0) / 100); }
function escapeHtml(value) { return String(value ?? '').replace(/[&<>"']/g, c => ({'&':'&amp;','<':'&lt;','>':'&gt;','"':'&quot;',"'":'&#39;'}[c])); }
function initials(name) { return String(name || '?').split(/\s+/).slice(0,2).map(x => x[0]).join('').toUpperCase(); }
function statusPill(status) { return `<span class="status-pill status-${escapeHtml(status)}">${escapeHtml(status)}</span>`; }
function severityClass(value) { return value >= 8 ? 'severity-high' : value >= 5 ? 'severity-medium' : 'severity-low'; }
let toastTimer;
function toast(message, error = false) {
  const node = $('#toast'); node.textContent = message; node.className = `toast show${error ? ' error' : ''}`;
  clearTimeout(toastTimer); toastTimer = setTimeout(() => node.className = 'toast', 3200);
}
function showPage(page) {
  if (!pageNames[page]) return;
  $$('.page').forEach(node => node.classList.toggle('active', node.id === `page-${page}`));
  $$('.nav-link').forEach(node => node.classList.toggle('active', node.dataset.page === page));
  $('#pageTitle').textContent = pageNames[page];
  $('#sidebar').classList.remove('open');
  if (page === 'overview') refreshOverview();
  if (page === 'patients') refreshPatients();
  if (page === 'queue') refreshQueue();
  if (page === 'doctors') refreshDoctors();
  if (page === 'beds') refreshBeds();
}
async function refreshDashboard() {
  const data = await api('/dashboard');
  $('#statTotal').textContent = data.totalPatients;
  $('#statWaiting').textContent = data.waitingPatients;
  $('#statAdmitted').textContent = data.admittedPatients;
  $('#statBeds').textContent = data.generalBedsFree + data.icuBedsFree;
  $('#statBedsNote').textContent = `${data.generalBedsFree} general · ${data.icuBedsFree} ICU available`;
  $('#queueCount').textContent = data.waitingPatients;
  $('#generalFree').textContent = `${data.generalBedsFree} / 3`;
  $('#icuFree').textContent = `${data.icuBedsFree} / 2`;
  $('#capacityBars').innerHTML = capacityBar('General ward', data.generalBedsFree, 3, false) + capacityBar('ICU ward', data.icuBedsFree, 2, true);
}
function capacityBar(label, free, total, icu) {
  const occupied = total - free, percent = Math.round(occupied / total * 100);
  return `<div class="capacity-item"><div><span>${label}</span><strong>${free} of ${total} free</strong></div><div class="progress ${icu ? 'icu' : ''}"><span style="width:${percent}%"></span></div></div>`;
}
async function loadPatients() { state.patients = await api('/patients'); return state.patients; }
function patientNameCell(p) { return `<div class="patient-cell"><span class="patient-avatar">${escapeHtml(initials(p.name))}</span><span><strong>${escapeHtml(p.name)}</strong><small>#${p.id}</small></span></div>`; }
function renderRecentPatients(items) {
  const rows = [...items].reverse().slice(0,5);
  $('#recentPatients').innerHTML = rows.length ? rows.map(p => `<tr><td>${patientNameCell(p)}</td><td>${escapeHtml(p.specialty)}</td><td><span class="severity ${severityClass(p.severity)}">${p.severity}/10</span></td><td>${statusPill(p.status)}</td></tr>`).join('') : '<tr><td colspan="4" class="empty-cell">No patients registered yet.</td></tr>';
}
function renderPatients() {
  const query = ($('#patientSearch')?.value || '').trim().toLowerCase();
  const filter = $('#patientFilter')?.value || 'all';
  const items = state.patients.filter(p => (filter === 'all' || p.status === filter) && [p.name, p.id, p.specialty].some(v => String(v).toLowerCase().includes(query)));
  $('#patientsTable').innerHTML = items.length ? items.map(p => `<tr><td>${patientNameCell(p)}</td><td>${escapeHtml(p.specialty)}</td><td><span class="severity ${severityClass(p.severity)}">${p.severity}/10</span></td><td>${p.bedId || '—'}</td><td>${p.doctorId || '—'}</td><td>${statusPill(p.status)}</td><td>${p.status === 'Admitted' ? `<button class="text-button" data-action="discharge" data-id="${p.id}">Discharge</button>` : p.status === 'Discharged' ? `<button class="text-button" data-action="view-bill" data-id="${p.id}">View bill</button>` : '<span class="muted">In queue</span>'}</td></tr>`).join('') : '<tr><td colspan="7" class="empty-cell">No matching patients found.</td></tr>';
}
async function refreshOverview() { try { await refreshDashboard(); await loadPatients(); renderRecentPatients(state.patients); } catch (e) { toast(e.message, true); } }
async function refreshPatients() { try { await loadPatients(); renderPatients(); } catch (e) { toast(e.message, true); } }
async function refreshQueue() {
  try {
    await loadPatients();
    const waiting = state.patients.filter(p => p.status === 'Waiting');
    $('#queueCount').textContent = waiting.length;
    $('#queueTable').innerHTML = waiting.length ? waiting.map((p, i) => `<tr><td><span class="soft-tag">#${i + 1}</span></td><td>${patientNameCell(p)}</td><td><span class="severity ${severityClass(p.severity)}">${p.severity}/10</span></td><td>${escapeHtml(p.specialty)}</td><td>${p.severity >= 8 ? '<span class="status-pill status-Waiting">ICU priority</span>' : '<span class="muted">Standard</span>'}</td></tr>`).join('') : '<tr><td colspan="5" class="empty-cell">The admission queue is empty.</td></tr>';
  } catch (e) { toast(e.message, true); }
}
async function refreshDoctors() {
  try {
    state.doctors = await api('/doctors');
    $('#doctorGrid').innerHTML = state.doctors.map(d => `<article class="doctor-card"><span class="doctor-avatar">${escapeHtml(initials(d.name))}</span><span class="doctor-details"><strong>${escapeHtml(d.name)}</strong><small>${escapeHtml(d.specialty)} · Doctor #${d.id}</small></span><span class="availability ${d.available ? '' : 'busy'}">${d.available ? 'Available' : `With #${d.patientId}`}</span></article>`).join('');
  } catch (e) { toast(e.message, true); }
}
async function refreshBeds() {
  try {
    const [beds, dashboard] = await Promise.all([api('/beds'), api('/dashboard')]); state.beds = beds;
    $('#generalFree').textContent = `${dashboard.generalBedsFree} / 3`; $('#icuFree').textContent = `${dashboard.icuBedsFree} / 2`;
    $('#bedGrid').innerHTML = beds.map(b => `<article class="bed-card ${b.occupied ? 'occupied' : ''}"><span class="bed-symbol">▤</span><strong>Bed ${b.id}</strong><small>${b.ward} ward</small><span class="bed-state">${b.occupied ? `Patient #${b.patientId}` : 'Available'}</span></article>`).join('');
  } catch (e) { toast(e.message, true); }
}
async function admitNext() {
  try { const result = await api('/admissions/next', { method: 'POST', body: '{}' }); toast(result.message); await refreshAll(); }
  catch (e) { toast(e.message, true); }
}
//async function dischargePatient(id) {
//  if (!confirm(`Discharge patient #${id}? This will close their bill and release their bed and doctor.`)) return;
//  try { const result = await api(`/patients/${id}/discharge`, { method: 'POST', body: '{}' }); toast(`${result.message} Total: ${money(result.totalPaise)}`); await refreshAll(); }
//  catch (e) { toast(e.message, true); }
//}


async function dischargePatient(id) {
    const confirmed = await showConfirmModal({
        title: "Discharge Patient",
        message: `Are you sure you want to discharge patient #${id}?`,
        details: "",
        confirmText: "Confirm",
        cancelText: "Cancel",
        danger: true
    });

    if (!confirmed) return;

    try {
        const result = await api(`/patients/${id}/discharge`, {
            method: 'POST',
            body: '{}'
        });

        toast(`${result.message} Total: ${money(result.totalPaise)}`);
        await refreshAll();
    }
    catch (e) {
        toast(e.message, true);
    }
}


function showConfirmModal({
    title,
    message,
    details,
    confirmText = "Confirm",
    cancelText = "Cancel",
    danger = false
}) {
    return new Promise((resolve) => {
        const modal = document.getElementById('confirmModal');

        document.getElementById('confirmTitle').textContent = title;
        document.getElementById('confirmMessage').textContent = message;
        document.getElementById('confirmDetails').textContent = details;

        const accept = document.getElementById('confirmAccept');
        const cancel = document.getElementById('confirmCancel');

        accept.textContent = confirmText;
        cancel.textContent = cancelText;

        accept.className = danger ? 'confirm-danger' : 'confirm-primary';

        const cleanup = (value) => {
            modal.close();
            accept.removeEventListener('click', onAccept);
            cancel.removeEventListener('click', onCancel);
            modal.removeEventListener('cancel', onEscape);
            resolve(value);
        };

        const onAccept = () => cleanup(true);
        const onCancel = () => cleanup(false);
        const onEscape = (event) => {
            event.preventDefault();
            cleanup(false);
        };

        accept.addEventListener('click', onAccept);
        cancel.addEventListener('click', onCancel);
        modal.addEventListener('cancel', onEscape);

        modal.showModal();
    });
}



async function openBill(id) {
  $('#billPatientId').value = id; showPage('billing');
  try {
    const bill = await api(`/bills/${id}`); state.currentBill = bill;
    const patient = bill.patient;
    $('#billResult').innerHTML = `<div class="bill-head"><div><strong>${escapeHtml(patient.name)}</strong><small>Patient #${patient.id} · ${escapeHtml(patient.status)}</small></div>${bill.closed ? '<span class="soft-tag">Closed</span>' : '<span class="status-pill status-Admitted">Active</span>'}</div>${bill.items.map(item => `<div class="bill-line"><span>${escapeHtml(item.description)}</span><strong>${money(item.amountPaise)}</strong></div>`).join('')}<div class="bill-total"><span>Total</span><strong>${money(bill.totalPaise)}</strong></div>${bill.closed ? '<p class="muted">This bill has been closed.</p>' : `<button class="button button-primary" data-action="add-charge" data-id="${id}">＋ Add charge</button>`}`;
  } catch (e) { $('#billResult').innerHTML = `<div class="bill-placeholder">${escapeHtml(e.message)}</div>`; toast(e.message, true); }
}
async function refreshAll() {
  await refreshDashboard();
  const activePage = $('.page.active')?.id.replace('page-', '');
  if (activePage === 'overview') { await loadPatients(); renderRecentPatients(state.patients); }
  if (activePage === 'patients') await refreshPatients();
  if (activePage === 'queue') await refreshQueue();
  if (activePage === 'doctors') await refreshDoctors();
  if (activePage === 'beds') await refreshBeds();
}
function openChargeModal(id) {
  const form = $('#chargeForm'); form.reset(); form.elements.patientId.value = id;
  $('#chargePatientLabel').textContent = `Patient #${id}`; $('#chargeDialog').showModal();
}

document.addEventListener('click', async event => {
  const pageButton = event.target.closest('[data-page]');
  if (pageButton) { showPage(pageButton.dataset.page); return; }
  const action = event.target.closest('[data-action]'); if (!action) return;
  switch (action.dataset.action) {
    case 'open-patient-modal': $('#patientDialog').showModal(); break;
    case 'close-patient-modal': $('#patientDialog').close(); break;
    case 'close-charge-modal': $('#chargeDialog').close(); break;
    case 'admit-next': await admitNext(); break;
    case 'discharge': await dischargePatient(Number(action.dataset.id)); break;
    case 'view-bill': await openBill(Number(action.dataset.id)); break;
    case 'add-charge': openChargeModal(Number(action.dataset.id)); break;
  }
});
$('#mobileMenu').addEventListener('click', () => $('#sidebar').classList.toggle('open'));
$('#patientSearch').addEventListener('input', renderPatients);
$('#patientFilter').addEventListener('change', renderPatients);
$('#patientForm').addEventListener('submit', async event => {
  event.preventDefault(); const form = event.currentTarget; const data = Object.fromEntries(new FormData(form));
  data.severity = Number(data.severity);
  try { const result = await api('/patients', { method: 'POST', body: JSON.stringify(data) }); $('#patientDialog').close(); form.reset(); toast(`${result.message} Patient ID: ${result.patient.id}`); await refreshAll(); showPage('patients'); }
  catch (e) { toast(e.message, true); }
});
$('#billLookupForm').addEventListener('submit', async event => { event.preventDefault(); await openBill(Number($('#billPatientId').value)); });
$('#chargeForm').addEventListener('submit', async event => {
  event.preventDefault(); const form = event.currentTarget; const id = Number(form.elements.patientId.value);
  const body = { description: form.elements.description.value.trim(), amountRupees: Number(form.elements.amountRupees.value) };
  try { const result = await api(`/bills/${id}/charges`, { method: 'POST', body: JSON.stringify(body) }); $('#chargeDialog').close(); toast(result.message); await openBill(id); }
  catch (e) { toast(e.message, true); }
});

$('#todayLabel').textContent = new Intl.DateTimeFormat('en-IN', { weekday: 'short', day: 'numeric', month: 'short', year: 'numeric' }).format(new Date());
refreshOverview();

// Admin logout
$('#adminLogout').addEventListener('click', () => {
    sessionStorage.removeItem('loggedInRole');
    window.location.href = '/admin-login.html';
});