/**
 * Image Modal Script for Ocular Documentation
 * Opens example images in a full-page modal when clicked
 */

(function() {
    'use strict';

    // Create modal HTML structure
    function createModal() {
        const modal = document.createElement('div');
        modal.id = 'imageModal';
        modal.innerHTML = `
            <div class="modal-overlay">
                <div class="modal-content">
                    <span class="modal-close">&times;</span>
                    <img class="modal-image" src="" alt="">
                    <div class="modal-caption"></div>
                </div>
            </div>
        `;
        return modal;
    }

    // Add CSS styles for the modal
    function addModalStyles() {
        const style = document.createElement('style');
        style.textContent = `
            #imageModal {
                display: none;
                position: fixed;
                z-index: 10000;
                left: 0;
                top: 0;
                width: 100%;
                height: 100%;
                background-color: rgba(0, 0, 0, 0.9);
                animation: fadeIn 0.3s ease;
            }

            #imageModal.show {
                display: flex;
                justify-content: center;
                align-items: center;
            }

            .modal-overlay {
                position: relative;
                width: 100%;
                height: 100%;
                display: flex;
                justify-content: center;
                align-items: center;
                padding: 20px;
                box-sizing: border-box;
            }

            .modal-content {
                position: relative;
                max-width: 95%;
                max-height: 95%;
                display: flex;
                flex-direction: column;
                align-items: center;
            }

            .modal-image {
                max-width: 100%;
                max-height: calc(100vh - 120px);
                object-fit: contain;
                border-radius: 8px;
                box-shadow: 0 4px 20px rgba(0, 0, 0, 0.5);
                animation: zoomIn 0.3s ease;
            }

            .modal-caption {
                color: white;
                font-size: 16px;
                font-weight: bold;
                margin-top: 15px;
                text-align: center;
                background-color: rgba(0, 0, 0, 0.7);
                padding: 8px 16px;
                border-radius: 4px;
            }

            .modal-close {
                position: absolute;
                top: -40px;
                right: -10px;
                color: white;
                font-size: 40px;
                font-weight: bold;
                cursor: pointer;
                z-index: 10001;
                transition: opacity 0.3s ease;
                user-select: none;
            }

            .modal-close:hover {
                opacity: 0.7;
            }

            @keyframes fadeIn {
                from { opacity: 0; }
                to { opacity: 1; }
            }

            @keyframes zoomIn {
                from { 
                    opacity: 0;
                    transform: scale(0.8);
                }
                to { 
                    opacity: 1;
                    transform: scale(1);
                }
            }

            /* Make images in sample sections clickable */
            .sample-images img {
                cursor: pointer;
                transition: transform 0.2s ease, box-shadow 0.2s ease;
            }

            .sample-images img:hover {
                transform: scale(1.02);
                box-shadow: 0 4px 12px rgba(0, 0, 0, 0.3);
            }
        `;
        document.head.appendChild(style);
    }

    // Initialize modal functionality
    function initModal() {
        // Add styles
        addModalStyles();

        // Create and add modal to page
        const modal = createModal();
        document.body.appendChild(modal);

        const modalElement = document.getElementById('imageModal');
        const modalImage = modal.querySelector('.modal-image');
        const modalCaption = modal.querySelector('.modal-caption');
        const closeBtn = modal.querySelector('.modal-close');

        // Function to open modal
        function openModal(imageSrc, caption) {
            modalImage.src = imageSrc;
            modalCaption.textContent = caption;
            modalElement.classList.add('show');
            document.body.style.overflow = 'hidden'; // Prevent background scrolling
        }

        // Function to close modal
        function closeModal() {
            modalElement.classList.remove('show');
            document.body.style.overflow = ''; // Restore scrolling
            // Clear image source after animation to prevent flicker
            setTimeout(() => {
                if (!modalElement.classList.contains('show')) {
                    modalImage.src = '';
                }
            }, 300);
        }

        // Add click handlers to all images in sample sections
        const sampleImages = document.querySelectorAll('.sample-images img');
        sampleImages.forEach(img => {
            img.addEventListener('click', function() {
                const caption = this.closest('.img-with-text')?.querySelector('p')?.textContent || 'Image';
                openModal(this.src, caption);
            });
        });

        // Close modal handlers
        closeBtn.addEventListener('click', closeModal);
        
        modalElement.addEventListener('click', function(e) {
            if (e.target === modalElement || e.target.classList.contains('modal-overlay')) {
                closeModal();
            }
        });

        // Close modal with Escape key
        document.addEventListener('keydown', function(e) {
            if (e.key === 'Escape' && modalElement.classList.contains('show')) {
                closeModal();
            }
        });
    }

    // Initialize when DOM is ready
    if (document.readyState === 'loading') {
        document.addEventListener('DOMContentLoaded', initModal);
    } else {
        initModal();
    }

})();
